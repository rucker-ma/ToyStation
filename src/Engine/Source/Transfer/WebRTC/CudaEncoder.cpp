//
// Created by ma on 2023/4/27.
//
#include "CudaEncoder.h"

#include <modules/video_coding/include/video_codec_interface.h>
#include <modules/video_coding/include/video_error_codes.h>

#include "Base/Macro.h"
#include "Base/Time.h"
#include "Base/Calculagraph.h"
#include "Device/Cuda/CudaPlatform.h"
#include "VideoSaver.h"
#include "ToyEngineSetting.h"

#define H264_NALU_SPS 0x67
#define H264_NALU_PPS 0x68
#define H264_NALU_IDR 0x65   //for idr frame,encoder will update sps pps
#define H264_NALU_I 0x61
#define H264_NALU_P 0x41
#define H264_NALU_B 0x01
#define H264_NALU_SEI 0x06
namespace toystation {
static Calculagraph encode_cal("Transfer FrameRate", 100, 1000);
static VideoSaver saver;
int CudaEncoder::InitEncode(const webrtc::VideoCodec* codec_settings,
                            const webrtc::VideoEncoder::Settings& settings) {
    if (!codec_settings ||
        codec_settings->codecType != webrtc::VideoCodecType::kVideoCodecH264) {
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }
    if (codec_settings->maxFramerate == 0) {
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }
    if (codec_settings->width < 1 || codec_settings->height < 1) {
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }
    if(ToyEngineSetting::Instance().SaveVideo()){
        saver.Init(codec_settings);
    }
    return InitNvEncoderCuda(codec_settings, settings);
}
int32_t CudaEncoder::RegisterEncodeCompleteCallback(
    webrtc::EncodedImageCallback* callback) {
    encoded_done_ = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}
int32_t CudaEncoder::Release() {
    if(ToyEngineSetting::Instance().SaveVideo()) {
        saver.End();
    }
    encoder_ = nullptr;
    return WEBRTC_VIDEO_CODEC_OK; }
int32_t CudaEncoder::Encode(
    const webrtc::VideoFrame& frame,
    const std::vector<webrtc::VideoFrameType>* frame_types) {
    if (!encoder_) {
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }
    if (frame_types->front() == webrtc::VideoFrameType::kEmptyFrame) {
        return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
    }

    const NvEncInputFrame* input_frame = encoder_->GetNextInputFrame();
    const webrtc::NV12BufferInterface* nv12_buffer =
        frame.video_frame_buffer()->GetNV12();
    if(CopyToEncodeFrame(*nv12_buffer,*input_frame)){
        std::vector<std::vector<uint8_t>> packets;
        if(rate_control_){
            NV_ENC_RECONFIGURE_PARAMS reconfigure_params = {NV_ENC_RECONFIGURE_PARAMS_VER};
            memcpy( &reconfigure_params.reInitEncodeParams,&initialize_params_,sizeof(NV_ENC_INITIALIZE_PARAMS));
            encode_config_.encodeCodecConfig.h264Config.disableSPSPPS = 1;
            encode_config_.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR;
            encode_config_.rcParams.averageBitRate = rate_control_->target_bitrate.get_sum_bps();
            if(!encoder_->Reconfigure(&reconfigure_params)){
                LogError("reconfigure encoder error");
            }
            rate_control_ = nullptr;
        }
//        if(frame_types->front() == webrtc::VideoFrameType::kVideoFrameKey){
//            LogInfo("WebRTC request encode key frame");
//            NV_ENC_PIC_PARAMS pic_params = {};
//            pic_params.pictureType = NV_ENC_PIC_TYPE_IDR;
//            encoder_->EncodeFrame(packets,&pic_params);
//        }else {
            encoder_->EncodeFrame(packets);
        //}
        if(packets.empty()){
            return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
        }
        for(auto& pkt:packets){
            debug::TimePiling::Instance().Mark("encode end",50);
            webrtc::EncodedImage image;
            image._encodedWidth = encoder_->GetEncodeWidth();
            image._encodedHeight = encoder_->GetEncodeHeight();
            image.SetTimestamp(frame.timestamp());
            image.SetSpatialIndex(0);
            assert(pkt.size()>5);
            if(pkt[4] == H264_NALU_IDR || pkt[4] == H264_NALU_I){ //65 IDR frame,61 I frame
                image._frameType =  webrtc::VideoFrameType::kVideoFrameKey;
                std::vector<uint8_t> spspps;
                encoder_->GetSequenceParams(spspps);
                if(ToyEngineSetting::Instance().SaveVideo()) {
                    saver.WritePacket(pkt, spspps);
                }
                spspps.insert(spspps.end(),pkt.begin(),pkt.end());
                auto buffer = webrtc::EncodedImageBuffer::Create(spspps.data(),spspps.size());
                image.SetEncodedData(buffer);

            }else{
                image._frameType = webrtc::VideoFrameType::kVideoFrameDelta;
                auto buffer = webrtc::EncodedImageBuffer::Create(pkt.data(),pkt.size());
                image.SetEncodedData(buffer);
                if(ToyEngineSetting::Instance().SaveVideo()) {
                    saver.WritePacket(pkt, {});
                }
            }
            webrtc::CodecSpecificInfo codec_spec;
            codec_spec.codecType = webrtc::kVideoCodecH264;
            codec_spec.codecSpecific.H264.packetization_mode =
                webrtc::H264PacketizationMode::NonInterleaved;
            codec_spec.codecSpecific.H264.temporal_idx = webrtc::kNoTemporalIdx;
            codec_spec.codecSpecific.H264.idr_frame = (pkt[4] == H264_NALU_IDR||pkt[4] == H264_NALU_I);

            debug::TimePiling::Instance().Mark("send to remote",60);
            encoded_done_->OnEncodedImage(image,&codec_spec);
            encode_cal.Step();
            std::string marks_info = debug::TimePiling::Instance().GetMarksInfo();
            if(!marks_info.empty()){
                LogInfo("one frame life cycle: \n" +  marks_info);
            }
        }
    }else{
        LogError("Copy Data to Encode Frame Error");
        return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
    }
    return WEBRTC_VIDEO_CODEC_OK;
}
void CudaEncoder::SetRates(
    const webrtc::VideoEncoder::RateControlParameters& parameters) {
    if(rate_control_ == nullptr){
        rate_control_ = std::make_shared<webrtc::VideoEncoder::RateControlParameters>(parameters);
    }
}
void CudaEncoder::OnPacketLossRateUpdate(float packet_loss_rate) {
    VideoEncoder::OnPacketLossRateUpdate(packet_loss_rate);
}
void CudaEncoder::OnRttUpdate(int64_t rtt_ms) {
    VideoEncoder::OnRttUpdate(rtt_ms);
}
void CudaEncoder::OnLossNotification(
    const webrtc::VideoEncoder::LossNotification& loss_notification) {
    VideoEncoder::OnLossNotification(loss_notification);
}
webrtc::VideoEncoder::EncoderInfo CudaEncoder::GetEncoderInfo() const {
    auto encoder_info = VideoEncoder::GetEncoderInfo();
    encoder_info.is_hardware_accelerated = true;
    return encoder_info;
}
int CudaEncoder::InitNvEncoderCuda(const webrtc::VideoCodec* codec_settings,
                                   const VideoEncoder::Settings& settings) {
    encoder_ = std::make_unique<NvEncoderCuda>(CudaPlatform::Instance().GetContext(),
                                               codec_settings->width,codec_settings->height,
                                               NV_ENC_BUFFER_FORMAT_NV12,2);
    initialize_params_ = { NV_ENC_INITIALIZE_PARAMS_VER };
    encode_config_ = { NV_ENC_CONFIG_VER };
    encode_config_.rcParams.rateControlMode = NV_ENC_PARAMS_RC_CBR;
    initialize_params_.encodeConfig = &encode_config_;

    encoder_->CreateDefaultEncoderParams(&initialize_params_,NV_ENC_CODEC_H264_GUID,NV_ENC_PRESET_P3_GUID,NV_ENC_TUNING_INFO_ULTRA_LOW_LATENCY);
    encoder_->CreateEncoder(&initialize_params_);

    encode_cal.OnEnd = [](double result,long long duration) {
        // LogDebug("Receive Render Frame,Process Time: " +
        //          std::to_string(static_cast<int>(duration)));

        LogInfo("Encode FrameRate: " + std::to_string((int)result));
    };
    encode_cal.Start();
    return WEBRTC_VIDEO_CODEC_OK;
}
bool CudaEncoder::CopyToEncodeFrame(const webrtc::NV12BufferInterface& buffer,
                       const NvEncInputFrame& inputframe){
    size_t data_size = buffer.width()*buffer.height();
    assert(buffer.width() == inputframe.pitch);
    cudaError res = cudaMemcpy(inputframe.inputPtr,buffer.DataY(),data_size,
                               cudaMemcpyDeviceToDevice);
    assert(res == CUDA_SUCCESS);
    res = cudaMemcpy((char*)inputframe.inputPtr+data_size,buffer.DataUV(),data_size/2,cudaMemcpyDeviceToDevice);
    return res == CUDA_SUCCESS;
}
}  // namespace toystation
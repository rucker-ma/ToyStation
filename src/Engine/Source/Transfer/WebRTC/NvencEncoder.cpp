//
// Created by ma on 2023/3/13.
//
#include "NvencEncoder.h"

#include <modules/video_coding/include/video_error_codes.h>
#include <modules/video_coding/include/video_codec_interface.h>

#include <cuda_runtime.h>

extern "C"{
#include <libavutil/hwcontext.h>

}
#include "Base/Macro.h"

namespace toystation{

NvencEncoder::NvencEncoder() {}
NvencEncoder::~NvencEncoder() {}
int NvencEncoder::InitEncode(const webrtc::VideoCodec *codec_settings,
                             const webrtc::VideoEncoder::Settings &settings) {
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

    return InitNvencEncoder(codec_settings, settings);
}
int32_t NvencEncoder::RegisterEncodeCompleteCallback(
    webrtc::EncodedImageCallback *callback) {
    encoded_done_ = callback;
    return WEBRTC_VIDEO_CODEC_OK;
}
int32_t NvencEncoder::Release() {
    assert(0);
    if(test_ptr) {
        cudaFree(test_ptr);
    }
    //TODO:完善资源的释放
    if(ctx_){
        avcodec_free_context(&ctx_);
        encoder_ = nullptr;
    }
    return WEBRTC_VIDEO_CODEC_OK;
}

int32_t NvencEncoder::Encode(
    const webrtc::VideoFrame &frame,
    const std::vector<webrtc::VideoFrameType> *frame_types) {
    if (!encoder_) {
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }
    if (!hw_frame_) {
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }
    if (frame_types->front() == webrtc::VideoFrameType::kEmptyFrame) {
        return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
    }
    if(!WriteToHWFrame(frame)){
        return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
    }
    AVPacket* packet = av_packet_alloc();
    int ret = avcodec_send_frame(ctx_, hw_frame_);
    if (ret < 0) {
        LogWarn("Send frame encoding error");
        av_packet_free(&packet);
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    while (ret >= 0) {
        ret = avcodec_receive_packet(ctx_, packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
            av_packet_free(&packet);
            return WEBRTC_VIDEO_CODEC_NO_OUTPUT;
        }
        if (ret < 0) {
            LogError("frame encoding failed,encodeframe return");
            av_packet_free(&packet);
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        webrtc::CodecSpecificInfo codec_spec;
        codec_spec.codecType = webrtc::kVideoCodecH264;
        codec_spec.codecSpecific.H264.packetization_mode =
            webrtc::H264PacketizationMode::NonInterleaved;
        codec_spec.codecSpecific.H264.temporal_idx = webrtc::kNoTemporalIdx;
        codec_spec.codecSpecific.H264.idr_frame =
            packet->flags == AV_PKT_FLAG_KEY;

        webrtc::EncodedImage image;
        image._encodedWidth = ctx_->width;
        image._encodedHeight = ctx_->height;
        image.SetSpatialIndex(0);
        image._frameType = (packet->flags == 1)
                               ? webrtc::VideoFrameType::kVideoFrameKey
                               : webrtc::VideoFrameType::kVideoFrameDelta;

        auto buffer = webrtc::EncodedImageBuffer::Create(packet->size);
        image.SetEncodedData(buffer);
        image.set_size(0);
        memcpy(buffer->data(), packet->data, packet->size);
        image.set_size(image.size() + packet->size);
        image.SetTimestamp((uint32_t)packet->pts);
        encoded_done_->OnEncodedImage(image, &codec_spec);

        av_packet_unref(packet);
    }
    av_packet_free(&packet);
    return WEBRTC_VIDEO_CODEC_OK;
}
void NvencEncoder::SetRates(
    const webrtc::VideoEncoder::RateControlParameters &parameters) {}
void NvencEncoder::OnPacketLossRateUpdate(float packet_loss_rate) {
    VideoEncoder::OnPacketLossRateUpdate(packet_loss_rate);
}
void NvencEncoder::OnRttUpdate(int64_t rtt_ms) {
    VideoEncoder::OnRttUpdate(rtt_ms);
}
void NvencEncoder::OnLossNotification(
    const webrtc::VideoEncoder::LossNotification &loss_notification) {
    VideoEncoder::OnLossNotification(loss_notification);
}
webrtc::VideoEncoder::EncoderInfo NvencEncoder::GetEncoderInfo() const {
    return VideoEncoder::GetEncoderInfo();
}
int NvencEncoder::InitNvencEncoder(
    const webrtc::VideoCodec *codec_settings,
    const webrtc::VideoEncoder::Settings &settings) {

    AVBufferRef* device_ref = nullptr;
    int ret = av_hwdevice_ctx_create(&device_ref,AV_HWDEVICE_TYPE_CUDA,"auto",NULL,0);
    if(ret<0){
        LogError("Cannot open cuda device");
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    encoder_ = avcodec_find_encoder_by_name("h264_nvenc");
    if(!encoder_){
        LogError("Not find h264_nvenc encoder");
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    ctx_ = avcodec_alloc_context3(encoder_);
    if(!ctx_){
        LogError("Cannot init AVCodecContext");
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    ctx_->width = codec_settings->width;
    ctx_->height = codec_settings->height;
    ctx_->bit_rate = codec_settings->maxBitrate * 1000;
//    ctx_->time_base = (AVRational){1, 25};
//    ctx_->framerate = (AVRational){25, 1};
    ctx_->pix_fmt = AV_PIX_FMT_CUDA;
    ctx_->time_base = {1, 90000};
    ctx_->hw_device_ctx = av_buffer_ref(device_ref);

    AVHWFramesContext* hwframe_ctx = NULL;
    AVBufferRef* hw_frames_ref = av_hwframe_ctx_alloc(device_ref);
    hwframe_ctx = (AVHWFramesContext*)(hw_frames_ref->data);
    hwframe_ctx->format = AV_PIX_FMT_CUDA;
    hwframe_ctx->sw_format=AV_PIX_FMT_NV12;
    hwframe_ctx->width = ctx_->width;
    hwframe_ctx->height = ctx_->height;
    ret =  av_hwframe_ctx_init(hw_frames_ref);
    if(ret<0){
        LogError("Failed to intialize Cuda Frame context");
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    ctx_->hw_frames_ctx = av_buffer_ref(hw_frames_ref);
    av_buffer_unref(&hw_frames_ref);

    ret = avcodec_open2(ctx_,encoder_,NULL);
    if(ret<0){
        LogError("Open Encoder error");
        avcodec_free_context(&ctx_);
        av_buffer_unref(&device_ref);
        return WEBRTC_VIDEO_CODEC_ERROR;
    }
    hw_frame_ = av_frame_alloc();
    ret = av_hwframe_get_buffer(ctx_->hw_frames_ctx,hw_frame_,0);
    if(ret<0){
        LogError("Allocate Hardware Frame Error");
    }
    cudaMalloc(&test_ptr,ctx_->width*ctx_->height);
    return WEBRTC_VIDEO_CODEC_OK;
}
bool NvencEncoder::WriteToHWFrame(const webrtc::VideoFrame& frame)
{
    const webrtc::NV12BufferInterface* nv12_buffer =
        frame.video_frame_buffer()->GetNV12();

    size_t yplane_size = nv12_buffer->width()*nv12_buffer->height();
    //BUG:此处需要使用cuda指针中转数据，直接拷贝会有bug，原因未知
    //TODO:取消数据中转和二次拷贝
    cudaError res = cudaMemcpy(test_ptr,nv12_buffer->DataY(),yplane_size,
                     cudaMemcpyDeviceToDevice);
    res = cudaMemcpy(hw_frame_->data[0],test_ptr,yplane_size,
               cudaMemcpyDeviceToDevice);

    res = cudaMemcpy(test_ptr,nv12_buffer->DataUV(),yplane_size/2,
                     cudaMemcpyDeviceToDevice);
    res = cudaMemcpy(hw_frame_->data[1],test_ptr,yplane_size/2,
                     cudaMemcpyDeviceToDevice);
    
    //cudaMemcpy(hw_frame_->data[0],nv12_buffer->DataY(),yplane_size,
    //           cudaMemcpyDeviceToDevice);
    //cudaMemcpy(hw_frame_->data[1],nv12_buffer->DataUV(),yplane_size/2,
    //           cudaMemcpyDeviceToDevice);
    //copy vulkan memory to frame buffer
    return true;
}
}
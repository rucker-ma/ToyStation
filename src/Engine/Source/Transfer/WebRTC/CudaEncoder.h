//
// Created by ma on 2023/4/27.
//

#pragma once

#include "VideoEncoder.h"
#include "Device/NvCodec/NvEncoderCuda.h"

namespace toystation{
class CudaEncoder:public webrtc::VideoEncoder {
public:
    CudaEncoder()=default;
    virtual ~CudaEncoder(){};
    // webrtc::VideoEncoder
    int InitEncode(const webrtc::VideoCodec* codec_settings,
                   const VideoEncoder::Settings& settings) override;
    int32_t RegisterEncodeCompleteCallback(
        webrtc::EncodedImageCallback* callback) override;
    int32_t Release() override;
    int32_t Encode(
        const webrtc::VideoFrame& frame,
        const std::vector<webrtc::VideoFrameType>* frame_types) override;
    void SetRates(const RateControlParameters& parameters) override;
    void OnPacketLossRateUpdate(float packet_loss_rate) override;
    void OnRttUpdate(int64_t rtt_ms) override;
    void OnLossNotification(const LossNotification& loss_notification) override;
    EncoderInfo GetEncoderInfo() const override;
private:
    int InitNvEncoderCuda(const webrtc::VideoCodec* codec_settings,
                         const VideoEncoder::Settings& settings);
    bool CopyToEncodeFrame(const webrtc::NV12BufferInterface& buffer,
                           const NvEncInputFrame& inputframe);
private:
    std::unique_ptr<NvEncoderCuda> encoder_;
    webrtc::EncodedImageCallback* encoded_done_;
    std::shared_ptr<RateControlParameters> rate_control_;
    NV_ENC_INITIALIZE_PARAMS initialize_params;
    NV_ENC_CONFIG encode_config;
};
}

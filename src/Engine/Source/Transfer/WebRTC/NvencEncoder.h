//
// Created by ma on 2023/3/13.
//
#pragma once


#include "VideoEncoder.h"

namespace toystation{

//wraper for nvidia h264(avc) encoder
class NvencEncoder:public webrtc::VideoEncoder{
public:
    NvencEncoder();
    virtual ~NvencEncoder();
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
    int InitNvencEncoder(const webrtc::VideoCodec* codec_settings,
                         const VideoEncoder::Settings& settings);
    bool WriteToHWFrame(const webrtc::VideoFrame& frame);
private:
    const AVCodec* encoder_;
    AVCodecContext* ctx_;
    webrtc::EncodedImageCallback* encoded_done_;
    AVFrame* hw_frame_;
    void* test_ptr=nullptr;
};

}

#pragma once

#include <api/video_codecs/video_encoder.h>
#include <media/base/codec.h>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/imgutils.h>
}

namespace toystation {
class ToyVideoEncoder : public webrtc::VideoEncoder {
public:
    virtual ~ToyVideoEncoder();
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
    int InitX264Encoder(const webrtc::VideoCodec* codec_settings,
                        const VideoEncoder::Settings& settings);

    AVCodec* encoder_;
    AVCodecContext* ctx_;
    AVFrame* encode_frame_;
    webrtc::EncodedImageCallback* encoded_done_;
};
}  // namespace toystation
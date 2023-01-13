#pragma once
#include <api/video_codecs/video_encoder_factory.h>
#include "VideoEncoder.h"

namespace toystation {
class ToyVideoEncoderFactory : public webrtc::VideoEncoderFactory {
public:
    virtual ~ToyVideoEncoderFactory(){};
    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;
    CodecInfo QueryVideoEncoder(
        const webrtc::SdpVideoFormat& format) const override;
    std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(
        const webrtc::SdpVideoFormat& format) override;
};
}  // namespace toystation
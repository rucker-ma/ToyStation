#pragma once
#include <api/video_codecs/video_encoder_factory.h>
#include <api/video_codecs/h264_profile_level_id.h>
//#include <api/video/video_stream_encoder_interface.h>
//#include <api/video/video_stream_encoder_observer.h>

#include "VideoEncoder.h"

namespace toystation {

webrtc::SdpVideoFormat CreateH264Format(webrtc::H264Profile profile,
                                        webrtc::H264Level level,
                                        const std::string& packetization_mode);
class ToyVideoEncoderFactory : public webrtc::VideoEncoderFactory {
public:
    virtual ~ToyVideoEncoderFactory(){};
    std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;
//    CodecInfo QueryVideoEncoder(
//        const webrtc::SdpVideoFormat& format) const override;
//    CodecSupport QueryVideoEncoder(
//        const webrtc::SdpVideoFormat& format,
//        absl::optional<std::string> scalability_mode) const override;

    std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(
        const webrtc::SdpVideoFormat& format) override;
};
}  // namespace toystation
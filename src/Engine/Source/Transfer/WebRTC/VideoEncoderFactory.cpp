#include "VideoEncoderFactory.h"

#include <media/base/codec.h>
#include <media/base/media_constants.h>

#include "VideoEncoder.h"


namespace toystation {

webrtc::SdpVideoFormat CreateH264Format(webrtc::H264Profile profile,
                                        webrtc::H264Level level,
                                        const std::string& packetization_mode) {
    // const absl::optional<std::string> profile_string =
    //     webrtc::H264ProfileLevelIdToString(
    //         webrtc::H264ProfileLevelId(profile, level));
    std::string profile_string = webrtc::H264ProfileLevelIdToString(
            webrtc::H264ProfileLevelId(profile, level)).value();
    return webrtc::SdpVideoFormat(
        cricket::kH264CodecName,
        {{cricket::kH264FmtpProfileLevelId,  profile_string},
         {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
         {cricket::kH264FmtpPacketizationMode, packetization_mode}});
}

std::vector<webrtc::SdpVideoFormat>
ToyVideoEncoderFactory::GetSupportedFormats() const {
    return {CreateH264Format(webrtc::H264Profile::kProfileBaseline,
                             webrtc::H264Level::kLevel3_1, "1"),
            CreateH264Format(webrtc::H264Profile::kProfileBaseline,
                             webrtc::H264Level::kLevel3_1, "0"),
            CreateH264Format(webrtc::H264Profile::kProfileConstrainedBaseline,
                             webrtc::H264Level::kLevel3_1, "1"),
            CreateH264Format(webrtc::H264Profile::kProfileConstrainedBaseline,
                             webrtc::H264Level::kLevel3_1, "0")};
}

webrtc::VideoEncoderFactory::CodecInfo
ToyVideoEncoderFactory::QueryVideoEncoder(
    const webrtc::SdpVideoFormat& format) const {
    return webrtc::VideoEncoderFactory::CodecInfo{};
}
std::unique_ptr<webrtc::VideoEncoder>
ToyVideoEncoderFactory::CreateVideoEncoder(
    const webrtc::SdpVideoFormat& format) {
    return std::make_unique<ToyVideoEncoder>();
}
}  // namespace toystation
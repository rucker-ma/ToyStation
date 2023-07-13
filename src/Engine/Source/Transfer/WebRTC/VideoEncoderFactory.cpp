#include "VideoEncoderFactory.h"
#include <sstream>
#include <media/base/codec.h>
#include <media/base/media_constants.h>

#include "VideoEncoder.h"
#include "NvencEncoder.h"
#include "CudaEncoder.h"

#include "ToyEngineSetting.h"

namespace toystation {

webrtc::SdpVideoFormat CreateH264Format(webrtc::H264Profile profile,
                                        webrtc::H264Level level,
                                        const std::string& packetization_mode) {
    std::string profile_str;
    //refer to https://www.rfc-editor.org/rfc/rfc6184#section-8.1
    switch (profile) {
        case webrtc::H264Profile::kProfileBaseline:
        case webrtc::H264Profile::kProfileConstrainedBaseline:
            profile_str = "42"; // "hex 42 "
            break ;
        case webrtc::H264Profile::kProfileMain:
            profile_str = "4d"; // "hex 4d "
            break ;
        case webrtc::H264Profile::kProfileHigh:
        case webrtc::H264Profile::kProfileConstrainedHigh:
            profile_str = "64"; // "hex 64 "
            break ;
        case webrtc::H264Profile::kProfilePredictiveHigh444:
            LogFatal("Nout support");
            break ;
    }
    switch (profile) {

        case webrtc::H264Profile::kProfileConstrainedBaseline:
        case webrtc::H264Profile::kProfileConstrainedHigh:
            profile_str += "e0";
            break ;
        case webrtc::H264Profile::kProfileBaseline:
        case webrtc::H264Profile::kProfileMain:
        case webrtc::H264Profile::kProfileHigh:
            profile_str += "00";
            break ;
        case webrtc::H264Profile::kProfilePredictiveHigh444:
            LogFatal("Nout support");
            break ;
    }
    std::ostringstream ss;
    ss<<std::hex<< static_cast<int>(level);
    if(ss.str().size() == 1){
        profile_str +='0';
    }
    profile_str +=ss.str();
//    std::string profile_string = webrtc::H264ProfileLevelIdToString(
//            webrtc::H264ProfileLevelId(profile, level)).value();

    return webrtc::SdpVideoFormat(
        cricket::kH264CodecName,
        {{cricket::kH264FmtpProfileLevelId,  profile_str},
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

//webrtc::VideoEncoderFactory::CodecInfo
//ToyVideoEncoderFactory::QueryVideoEncoder(
//    const webrtc::SdpVideoFormat& format) const {
//    return webrtc::VideoEncoderFactory::CodecInfo{};
//}
std::unique_ptr<webrtc::VideoEncoder>
ToyVideoEncoderFactory::CreateVideoEncoder(
    const webrtc::SdpVideoFormat& format) {

    if(ToyEngineSetting::Instance().GetUseHWAccel()){
        return std::make_unique<CudaEncoder>();
    }else{
        return std::make_unique<ToyVideoEncoder>();
    }
}
}  // namespace toystation
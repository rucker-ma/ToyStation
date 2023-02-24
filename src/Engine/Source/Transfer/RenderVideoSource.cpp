#include "RenderVideoSource.h"

#include <third_party/libyuv/include/libyuv.h>

#include "File/FileUtil.h"
#include "WebRTC/RTCI420Buffer.h"


namespace toystation {
RenderVideoSource::~RenderVideoSource() {}

void RenderVideoSource::Initialize() {
    RenderEvent::OnRenderDone = [this](std::shared_ptr<RenderFrame> frame) {
        ReceiveFrame(frame);
    };
}

void RenderVideoSource::ReceiveFrame(std::shared_ptr<RenderFrame> frame) {
    // LogDebug("Get One Frame");

    if (frame->Type() != RenderFrameType::FRAME_YCbCr) {
        return;
    }
    std::shared_ptr<RenderFrameYCbCr> frame_yuv =
        std::dynamic_pointer_cast<RenderFrameYCbCr>(frame);

    i420_buffer_ = RTCI420Buffer::Create(frame_yuv);
    // FileUtil::WriteBmp("test.bmp",frame.Data(),frame.Width(),frame.Height());
    // TODO:优化格式转换
    // libyuv::ABGRToI420(frame.Data(), frame_width * 4,
    //                    i420_buffer_->MutableDataY(), i420_buffer_->StrideY(),
    //                    i420_buffer_->MutableDataU(), i420_buffer_->StrideU(),
    //                    i420_buffer_->MutableDataV(), i420_buffer_->StrideV(),
    //                    frame_width, frame_height);

    OnFrame(webrtc::VideoFrame(i420_buffer_, webrtc::kVideoRotation_0, 0));
}
bool RenderVideoSource::remote() const { return false; }
absl::optional<bool> RenderVideoSource::needs_denoising() const {
    return false;
}
webrtc::MediaSourceInterface::SourceState RenderVideoSource::state() const {
    return webrtc::MediaSourceInterface::kLive;
}
bool RenderVideoSource::is_screencast() const { return true; }
}  // namespace toystation
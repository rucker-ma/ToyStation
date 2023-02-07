#include "RenderVideoSource.h"

#include <third_party/libyuv/include/libyuv.h>

#include "File/FileUtil.h"
namespace toystation {
RenderVideoSource::~RenderVideoSource() {}

void RenderVideoSource::Initialize() {
    RenderEvent::OnRenderDone = [this](const RenderFrame& frame){
        ReceiveFrame(frame);
    };
}

void RenderVideoSource::ReceiveFrame(const RenderFrame& frame) {
    // LogDebug("Get One Frame");
    int frame_width =
        frame.Width() > INT_MAX ? 0 : static_cast<int>(frame.Width());
    int frame_height =
        frame.Height() > INT_MAX ? 0 : static_cast<int>(frame.Height());

    if (!i420_buffer_.get()) {
        i420_buffer_ = webrtc::I420Buffer::Create(frame_width, frame_height);
    }
    // FileUtil::WriteBmp("test.bmp",frame.Data(),frame.Width(),frame.Height());
    libyuv::ABGRToI420(frame.Data(), frame_width * 4,
                       i420_buffer_->MutableDataY(), i420_buffer_->StrideY(),
                       i420_buffer_->MutableDataU(), i420_buffer_->StrideU(),
                       i420_buffer_->MutableDataV(), i420_buffer_->StrideV(),
                       frame_width, frame_height);

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
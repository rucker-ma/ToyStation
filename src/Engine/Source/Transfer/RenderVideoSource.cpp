#include "RenderVideoSource.h"

#include <third_party/libyuv/include/libyuv.h>
#include "File/FileUtil.h"
namespace toystation {
RenderVideoSource::~RenderVideoSource() {}

void RenderVideoSource::Initialize() {
    RenderEvent::OnRenderDone = std::bind(&RenderVideoSource::ReceiveFrame,
                                          this, std::placeholders::_1);
}

void RenderVideoSource::ReceiveFrame(const RenderFrame& frame) {
    LogDebug("Get One Frame");

    if (!i420_buffer_.get()) {
        i420_buffer_ =
            webrtc::I420Buffer::Create(frame.Width(), frame.Height());
    }
 
    //FileUtil::WriteBmp("test.bmp",frame.Data(),frame.Width(),frame.Height());
    
    libyuv::BGRAToI420(frame.Data(), frame.Width() * 4,
                       i420_buffer_->MutableDataY(), i420_buffer_->StrideY(),
                       i420_buffer_->MutableDataU(), i420_buffer_->StrideU(),
                       i420_buffer_->MutableDataV(), i420_buffer_->StrideV(),
                       frame.Width(), frame.Height());

    OnFrame(webrtc::VideoFrame(i420_buffer_, webrtc::kVideoRotation_0, 0));

    // OnFrame();
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
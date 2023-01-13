#include "RenderVideoSource.h"
namespace toystation
{
RenderVideoSource::~RenderVideoSource() {}

void RenderVideoSource::Initialize() {

}

void RenderVideoSource::ReceiveFrame(RenderFrame& frame) {}
bool RenderVideoSource::remote() const { return false; }
absl::optional<bool> RenderVideoSource::needs_denoising() const {
    return false;
}
webrtc::MediaSourceInterface::SourceState RenderVideoSource::state() const {
    return webrtc::MediaSourceInterface::kLive;
}
bool RenderVideoSource::is_screencast() const { return true; }
}  // namespace toystation
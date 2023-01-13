#pragma once
#include <media/base/adapted_video_track_source.h>

#include "Render/RenderContext.h"

namespace toystation {
class RenderVideoSource : public rtc::AdaptedVideoTrackSource {
public:
    virtual ~RenderVideoSource();
    void Initialize();
    void ReceiveFrame(RenderFrame& frame);
    // implement AdaptedVideoTrackSource
    bool remote() const override;
    absl::optional<bool> needs_denoising() const override;
    webrtc::MediaSourceInterface::SourceState state() const override;
    bool is_screencast() const override;
};
}  // namespace toystation
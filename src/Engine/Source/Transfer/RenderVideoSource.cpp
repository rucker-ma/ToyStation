#include "RenderVideoSource.h"
#include "Base/Time.h"
#include "Base/Calculagraph.h"
#include "File/FileUtil.h"
#include "Render/RenderFrame.h"
#include "WebRTC/RTCVideoFrameBuffer.h"

namespace toystation {


RenderVideoSource::~RenderVideoSource() {}

void RenderVideoSource::Initialize() {
    RenderEvent::OnRenderDone = [this](std::shared_ptr<RenderFrame> frame) {
        ReceiveFrame(frame);
    };
}

void RenderVideoSource::ReceiveFrame(std::shared_ptr<RenderFrame> frame) {
    // LogDebug("Get One Frame");
    debug::TimePiling::Instance().Mark("get render frame",20);
    if (frame->Type() == RenderFrameType::FRAME_YCbCr) {
        std::shared_ptr<RenderFrameYCbCr> frame_yuv =
            std::dynamic_pointer_cast<RenderFrameYCbCr>(frame);
        video_buffer_ = RTCI420Buffer::Create(frame_yuv);
    }
    else if(frame->Type() == RenderFrameType::FRAME_NV12){
        std::shared_ptr<RenderFrameNV12> frame_nv12 =
            std::dynamic_pointer_cast<RenderFrameNV12>(frame);
        video_buffer_ = RTCNV12Buffer::Create(frame_nv12);
    }else{
        return;
    }
    // FileUtil::WriteBmp("test.bmp",frame.Data(),frame.Width(),frame.Height());
    debug::TimePiling::Instance().Mark("begin send to encoder",30);
    OnFrame(webrtc::VideoFrame(video_buffer_, webrtc::kVideoRotation_0, 0));
}
bool RenderVideoSource::remote() const { return false; }
absl::optional<bool> RenderVideoSource::needs_denoising() const {
    return false;
}
webrtc::MediaSourceInterface::SourceState RenderVideoSource::state() const {
    return webrtc::MediaSourceInterface::kLive;
}
bool RenderVideoSource::is_screencast() const { return false; }
}  // namespace toystation
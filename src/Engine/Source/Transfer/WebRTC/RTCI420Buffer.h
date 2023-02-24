#pragma once
#include <api/video/i420_buffer.h>

#include "Render/RenderContext.h"

namespace toystation {
class RTCI420Buffer : public webrtc::I420BufferInterface {
public:
    static rtc::scoped_refptr<RTCI420Buffer> Create(
        std::shared_ptr<RenderFrameYCbCr> frame);
    int width() const override;
    int height() const override;
    const uint8_t* DataY() const override;
    const uint8_t* DataU() const override;
    const uint8_t* DataV() const override;
    int StrideY() const override;
    int StrideU() const override;
    int StrideV() const override;

protected:
    RTCI420Buffer(std::shared_ptr<RenderFrameYCbCr> frame);
    virtual ~RTCI420Buffer() = default;

private:
    std::shared_ptr<RenderFrameYCbCr> frame_;
};
}  // namespace toystation
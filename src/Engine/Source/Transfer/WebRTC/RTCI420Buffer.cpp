#include "RTCI420Buffer.h"

#include <rtc_base/ref_counted_object.h>

namespace toystation {
rtc::scoped_refptr<RTCI420Buffer> RTCI420Buffer::Create(
    std::shared_ptr<RenderFrameYCbCr> frame) {
    return new rtc::RefCountedObject<RTCI420Buffer>(frame);
}

int RTCI420Buffer::width() const { return frame_->Width(); }
int RTCI420Buffer::height() const { return frame_->Height(); }
const uint8_t* RTCI420Buffer::DataY() const { return frame_->DataY(); }
const uint8_t* RTCI420Buffer::DataU() const { return frame_->DataCb(); }
const uint8_t* RTCI420Buffer::DataV() const { return frame_->DataCr(); }
int RTCI420Buffer::StrideY() const { return frame_->Width(); }
int RTCI420Buffer::StrideU() const { return frame_->Width() / 2; }
int RTCI420Buffer::StrideV() const { return frame_->Width() / 2; }
RTCI420Buffer::RTCI420Buffer(std::shared_ptr<RenderFrameYCbCr> frame)
    :  frame_(frame) {}
}  // namespace toystation
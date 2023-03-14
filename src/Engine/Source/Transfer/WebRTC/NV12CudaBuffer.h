//
// Created by ma on 2023/3/13.
//

#pragma once
#include <api/video/nv12_buffer.h>

namespace toystation{

class NV12CudaBuffer:public webrtc::NV12BufferInterface{
public:
    const uint8_t* DataY() const override;
    const uint8_t* DataUV() const override;
};
}
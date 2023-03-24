//
// Created by ma on 2023/3/13.
//
#pragma once
#include "RenderContext.h"

namespace toystation{

//将处理后的VkImage映射到Cuda Pointer,便于直接硬件加速编码

class CudaExternalMemory;
class RenderFrameNV12:public RenderFrame{
public:
    RenderFrameNV12(std::shared_ptr<RenderContext> context,
                    const RHIImage& compy, const RHIImage& compuv);
    virtual ~RenderFrameNV12();
    unsigned char* DataY()const;
    unsigned char* DataUV()const;
    unsigned int Width() const override;
    unsigned int Height() const override;
    RenderFrameType Type() const override;
private:
    void ReadRHIImage(RHIBuffer& buf, const RHIImage& img,
                 VkDeviceSize mem_size,
                 VkExtent2D img_size);
private:
    unsigned int width_;
    unsigned int height_;
    std::shared_ptr<CudaExternalMemory> cuda_mem_y_;
    std::shared_ptr<CudaExternalMemory> cuda_mem_uv_;
    RHIBuffer buffer_y_;
    RHIBuffer buffer_uv_;
    std::shared_ptr<RenderContext> context_;
};
}

#pragma once
#include <functional>
#include <map>
#include <memory>

#include "Vulkan/CommandPool.h"
#include "Vulkan/ResourceAllocator.h"
#include "Vulkan/SwapChain.h"
#include "Vulkan/VkContext.h"

namespace toystation {

struct RenderContextCreateInfo {
    enum RenderMode { RENDER_LOCAL, RENDER_REMOTE, RENDER_INVALID };

    RenderMode mode;
    static std::map<RenderMode, std::string> kRenderModeStr;
};

template <typename T>
std::string ToStr(T type) {
    return "";
}

template <>
std::string ToStr(RenderContextCreateInfo::RenderMode mode);

class RenderContext {
public:
    void Initialize(RenderContextCreateInfo& info);

    std::shared_ptr<VkContext> GetContext() const;
    std::shared_ptr<SwapChainBase> GetSwapchain() const;
    std::shared_ptr<VkResourceAllocator> GetAllocator() const;
    std::shared_ptr<CommandPool> GetCommandPool() const;

    static void SaveToImage(std::string filename, RHIImage& image,
                            VkImageLayout layout, VkFormat format,
                            std::shared_ptr<RenderContext> context);

private:
    std::shared_ptr<VkContext> vkctx_;
    std::shared_ptr<SwapChainBase> swapchain_;
    std::shared_ptr<DedicatedResourceAllocator> allocator_;
    std::shared_ptr<CommandPool> cmd_pool_;
};
enum RenderFrameType {
    FRAME_RED,FRAME_RG,
    FRAME_RGB, FRAME_RGBA, FRAME_YCbCr,FRAME_NV12};

class RenderFrame {
public:
    virtual ~RenderFrame() {}
    virtual unsigned char* Data() const { return nullptr; }
    virtual unsigned int Width() const { return 0; }
    virtual unsigned int Height() const { return 0; }
    virtual RenderFrameType Type() const { return RenderFrameType::FRAME_RGB; }
};
//将RGBA格式VkImage映射到CPU内存空间data指针（大小：width*height*4）
class RenderFrameImpl : public RenderFrame {
public:
    RenderFrameImpl() = delete;
    RenderFrameImpl(const RenderFrameImpl& frame) = delete;
    RenderFrameImpl& operator=(const RenderFrameImpl& frame) = delete;

    RenderFrameImpl(std::shared_ptr<RenderContext> context,
                    const RHIImage& img);
    virtual ~RenderFrameImpl();
    virtual unsigned char* Data() const;
    virtual unsigned int Width() const;
    virtual unsigned int Height() const;
    virtual RenderFrameType Type() const;

private:
    RHIBuffer buf_;
    std::shared_ptr<RenderContext> context_;
    unsigned int width_;
    unsigned int height_;
    unsigned char* data_;
};

//将Comupute shader处理后的Y,Cb,Cr三个VkImage分别映射到CPU空间DataY(width*height) ,
// DataCb(1/4 DataY Size),DataCr(1/4 DataY Size)
class RenderFrameYCbCr : public RenderFrame {
public:
    RenderFrameYCbCr(std::shared_ptr<RenderContext> context,
                     const RHIImage& compy, const RHIImage& compcb,
                     const RHIImage& compcr);
    virtual ~RenderFrameYCbCr();
    virtual unsigned char* Data() const;

    unsigned char* DataY() const;
    unsigned char* DataCb() const;
    unsigned char* DataCr() const;

    virtual unsigned int Width() const;
    virtual unsigned int Height() const;
    virtual RenderFrameType Type() const;

private:
    unsigned char* ReadRHIImage(RHIBuffer& buf, const RHIImage& img,
                                VkDeviceSize mem_size, VkExtent2D img_size);

private:
    std::shared_ptr<RenderContext> context_;
    unsigned int width_;
    unsigned int height_;
    RHIBuffer bufy_;
    RHIBuffer bufu_;
    RHIBuffer bufv_;
    unsigned char* datay_;
    unsigned char* datau_;
    unsigned char* datav_;
};


class RenderEvent {
public:
    static std::function<void(std::shared_ptr<RenderFrame>)> OnRenderDone;
};
}  // namespace toystation
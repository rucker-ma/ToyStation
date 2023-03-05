#pragma once
#include <functional>
#include <map>
#include <memory>

#include "Render/RenderCamera.h"
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
    std::shared_ptr<RenderCamera> GetCamera() const;
    static void SaveToImage(std::string filename, RHIImage& image,
                            VkImageLayout layout, VkFormat format,
                            std::shared_ptr<RenderContext> context);

private:
    std::shared_ptr<VkContext> vkctx_;
    std::shared_ptr<SwapChainBase> swapchain_;
    std::shared_ptr<DedicatedResourceAllocator> allocator_;
    std::shared_ptr<CommandPool> cmd_pool_;
    std::shared_ptr<RenderCamera> camera_;
};
enum RenderFrameType { FRAME_RGB, FRAME_RGBA, FRAME_YCbCr };

class RenderFrame {
public:
    virtual ~RenderFrame() {}
    virtual unsigned char* Data() const { return nullptr; }
    virtual unsigned int Width() const { return 0; }
    virtual unsigned int Height() const { return 0; }
    virtual RenderFrameType Type() const { return RenderFrameType::FRAME_RGB; }
};

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
    Buffer buf_;
    std::shared_ptr<RenderContext> context_;
    unsigned int width_;
    unsigned int height_;
    unsigned char* data_;
};

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
    unsigned char* ReadRHIImage(Buffer& buf, const RHIImage& img,
                                VkDeviceSize mem_size, VkExtent2D img_size);

private:
    std::shared_ptr<RenderContext> context_;
    unsigned int width_;
    unsigned int height_;
    Buffer bufy_;
    Buffer bufu_;
    Buffer bufv_;
    unsigned char* datay_;
    unsigned char* datau_;
    unsigned char* datav_;
};

class RenderEvent {
public:
    static std::function<void(std::shared_ptr<RenderFrame>)> OnRenderDone;
};
}  // namespace toystation
//
// Created by ma on 2023/3/13.
//

#pragma once
#include <memory>
#include <string>
#include <vector>

#ifdef TOYSTATION_CUDA
#include <cuda_runtime.h>
#endif
#include "Vulkan/ResourceAllocator.h"

namespace toystation {

class CudaPlatform {
public:
    static CudaPlatform& Instance();
    bool IsSupported();

private:
    CudaPlatform();
    void Init();

private:
    struct CudaDeviceInfo {
        int count;
        std::vector<std::string> names;
        std::vector<std::string> driver_version;
        std::vector<std::string> capability;
    };
    CudaDeviceInfo device_info_;
};

#ifdef TOYSTATION_CUDA  // implement cuda method
class CudaExternalMemory {
public:
    static std::shared_ptr<CudaExternalMemory> FromVulkanExternalMemory(
        std::shared_ptr<VkContext> context, Buffer& buffer);
    void* Data()const;
private:
    CudaExternalMemory();
    ~CudaExternalMemory();
    void ImportMemory(std::shared_ptr<VkContext> context, Buffer& vkmemory);

    class deleter;
    friend class deleter;
    class deleter {
    public:
        void operator()(CudaExternalMemory* mem) { delete mem; }
    };

private:
    void* cuda_ptr_;
    cudaExternalMemory_t cuda_mem_;
};
#endif

}  // namespace toystation
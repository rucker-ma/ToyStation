//
// Created by ma on 2023/3/13.
//
#include "CudaPlatform.h"

#include "Base/Macro.h"

namespace toystation {
CudaPlatform& CudaPlatform::Instance() {
    static CudaPlatform cuda_platform;
    return cuda_platform;
}
bool CudaPlatform::IsSupported() { return device_info_.count != 0; }
CudaPlatform::CudaPlatform() {
    device_info_.count = 0;
    Init();
}

void CudaPlatform::Init() {
#ifdef TOYSTATION_CUDA
    if (cudaGetDeviceCount(&device_info_.count) != cudaSuccess) {
        LogInfo("Found Cuda Device Error");
        return;
    }
    if (device_info_.count == 0) {
        LogWarn("No device support CUDA");
        return;
    }
    int major = 0, minor = 0;
    for (int index = 0; index < device_info_.count; ++index) {
        cudaSetDevice(index);
        cudaDeviceProp device_prop;
        cudaGetDeviceProperties(&device_prop,index);
        device_info_.names.push_back(std::string(device_prop.name));
        device_info_.capability.push_back(std::to_string(device_prop.major) + "." +
                                          std::to_string(device_prop.minor));
        int driver_version = 0;
        cudaDriverGetVersion(&driver_version);
        device_info_.driver_version.push_back(
            std::to_string(driver_version / 1000) + "." +
            std::to_string((driver_version % 100) / 10));
    }
#endif
}


#if TOYSTATION_CUDA
std::shared_ptr<CudaExternalMemory> CudaExternalMemory::FromVulkanExternalMemory(std::shared_ptr<VkContext>context,
                                                             Buffer& buffer){
    std::shared_ptr<CudaExternalMemory> mem(new CudaExternalMemory(),CudaExternalMemory::deleter());
    mem->ImportMemory(context,buffer);
    return mem;
}
CudaExternalMemory::CudaExternalMemory():cuda_ptr_(nullptr),cuda_mem_(nullptr){}
CudaExternalMemory::~CudaExternalMemory(){
    if(cuda_mem_){
        cudaDestroyExternalMemory(cuda_mem_);
        cudaFree(cuda_ptr_);
    }
}
void CudaExternalMemory::ImportMemory(std::shared_ptr<VkContext>context, Buffer& vkmemory){
    cudaExternalMemoryHandleDesc external_handle_desc = {};
    external_handle_desc.type = cudaExternalMemoryHandleTypeOpaqueWin32;
    external_handle_desc.size = MemHandleUtils::GetSize(vkmemory.handle);
    external_handle_desc.handle.win32.handle = MemHandleUtils::GetExternalWin32Handle(context->GetDevice(),vkmemory.handle);
    cudaError res = cudaImportExternalMemory(&cuda_mem_,&external_handle_desc);
    if(res !=cudaSuccess){
        LogError("cuda import external memory error");
        return;
    }
    cudaExternalMemoryBufferDesc external_buffer_desc={};
    external_buffer_desc.offset=0;
    external_buffer_desc.size =external_handle_desc.size;
    external_buffer_desc.flags=0;
    res = cudaExternalMemoryGetMappedBuffer(&cuda_ptr_,cuda_mem_,&external_buffer_desc);
    if(res !=cudaSuccess){
        LogError("cuda map external memory error");
    }
}
void* CudaExternalMemory::Data() const {
    return cuda_ptr_;
}

#endif

}  // namespace toystation
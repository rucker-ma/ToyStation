//
// Created by ma on 2023/3/16.
//
#include "ToyEngineSetting.h"
#include "Device/Cuda/CudaPlatform.h"

namespace toystation{
ToyEngineSetting& ToyEngineSetting::Instance(){
    static ToyEngineSetting setting;
    return setting;
}
void ToyEngineSetting::SetUseHWAccel(bool value) {
#ifdef TOYSTATION_CUDA
    if(CudaPlatform::Instance().IsSupported()) {
        use_hwaccel_ = value;
    }else{
        LogWarn("Hardware device not support");
    }
#endif
}
bool ToyEngineSetting::GetUseHWAccel() { return use_hwaccel_; }
bool ToyEngineSetting::SaveVideo(){return false;}
}
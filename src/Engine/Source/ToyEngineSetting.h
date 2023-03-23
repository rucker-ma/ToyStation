//
// Created by ma on 2023/3/16.
//

#pragma once

#define DEFINE_PROPS(type, name) \
private:                         \
    type name##_;                \
                                 \
public:                          \
    void Set##name(type value);  \
    type Get##name();

#define IMPLE_PROPS(type, name, cla)                     \
    void cla::Set##name(type value) { name##_ = value; } \
    type cla::Get##name() { return name##_; }

namespace toystation {

class ToyEngineSetting {

public:
    static ToyEngineSetting& Instance();
    //when set true, will use hardware encode,depend on cuda(help transfer vulkan
    //render frame to nvenc encode buffer)

    void SetUseHWAccel(bool value);
    bool GetUseHWAccel();
private:
    ToyEngineSetting() = default;

    bool use_hwaccel_ = false;
};

}  // namespace toystation

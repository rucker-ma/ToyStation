//
// Created by ma on 2023/3/5.
//
#include "InputSystem.h"
#include <cassert>

namespace toystation{


void InputSystem::Initialize() {}
void InputSystem::Tick() {}
void InputSystem::OnKey(InputAction action,int keycode) {
    std::lock_guard<std::mutex> lck(mtx);
    auto iter= std::find(pressed_keys_.begin(),
                              pressed_keys_.end(),keycode);
    switch (action) {
        case PRESS:
            assert(iter == pressed_keys_.end());
            pressed_keys_.push_back(keycode);
        case RELEASE:
            assert(iter!=pressed_keys_.end());
            pressed_keys_.erase(iter);
    }
}
void InputSystem::OnCursorPosition() {}
void InputSystem::OnFocus(bool enable) {}
bool InputSystem::IsFoucs() { return true; }
bool InputSystem::IsPress(int keycode){
    std::lock_guard<std::mutex> lck(mtx);
    return std::find(pressed_keys_.begin(),pressed_keys_.end(),keycode)
           !=pressed_keys_.end();
}
}
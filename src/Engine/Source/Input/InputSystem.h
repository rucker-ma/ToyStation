//
// Created by ma on 2023/3/5.
//
#pragma once
#include <list>
#include <string>
#include <mutex>

namespace toystation{

enum InputAction{
    PRESS,
    RELEASE
};

class InputSystem{
public:
    void Initialize();
    void Tick();
    //Tigger function
    //contain keyboard key and mouse key
    void OnKey(InputAction action,int keycode);
    void OnCursorPosition();
    void OnFocus(bool enable);

    bool IsFoucs();
    bool IsPress(int keycode);
private:
    //TODO:record key press time
    std::list<int> pressed_keys_;
    std::mutex mtx;
};
}

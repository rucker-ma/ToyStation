//
// Created by ma on 2023/3/5.
//
#pragma once
#include <list>
#include <string>
#include <mutex>

namespace toystation{

constexpr int KEY_A = 65;
constexpr int KEY_D = 68;
constexpr int KEY_E = 69;
constexpr int KEY_S = 83;
constexpr int KEY_Q = 81;
constexpr int KEY_W = 87;

constexpr int KEY_INVALID = 0;

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
    //consider multi key pressed,now only assume one key for move
    int PressedKey();
private:
    //TODO:record key press time
    std::list<int> pressed_keys_;
    std::mutex mtx;
};
}

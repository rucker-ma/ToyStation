//
// Created by ma on 2023/3/5.
//
#pragma once
#include <list>
#include <string>
#include <mutex>
#include <map>
#include <future>

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
    RELEASE,
    MOUSEMOVE
};

class InputSystem{
public:
    void Initialize();
    void Tick();
    //Tigger function
    //contain keyboard key and mouse key
    void OnKey(InputAction action,int keycode);
    void OnMoveDelta(int x,int y);
    std::pair<int,int> GetMoveDelta();
    void OnFocus(bool enable);

    bool IsFoucs();
    bool IsPress(int keycode);
    //consider multi key pressed,now only assume one key for move
    int PressedKey();
    int TakeoutOneKey(int keycode);
    void ProcessRemoteMessage();
    void OnInputMessage(std::string msg);
private:
    //TODO:record key press time
    std::list<int> pressed_keys_;
    std::map<int,int> pressed_counter_;
    std::list<std::pair<int,int>> move_deltas_;
    std::mutex mtx;
    std::condition_variable cv_;
    std::list<std::string> msgs_;
    std::future<void> process_input_future_;
};
}

//
// Created by ma on 2023/3/5.
//
#pragma once
#include <list>
#include <string>
#include <mutex>
#include <map>
#include <future>
#include "File/FileUtil.h"

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
    DRAG_MOVE, //鼠标左键按下拖动
    LOCK_MOVE //鼠标pointerlock 拖动
};

class InputSystem{
public:
    void Initialize();
    void Tick();
    //Tigger function
    //contain keyboard key and mouse key
    void OnKey(InputAction action,int keycode);
    std::pair<int,int> GetMoveDelta(InputAction action);

    int TakeoutOneKey(int keycode);
    void OnInputMessage(std::string msg);
private:
    std::pair<int,int> GetMove(std::list<std::pair<int,int>>& container);
private:
    //TODO:record key press time
    std::list<int> pressed_keys_;
    std::map<int,int> pressed_counter_;
    std::list<std::pair<int,int>> drag_mvoe_;
    std::list<std::pair<int,int>> lock_move_;
    std::mutex mtx;
    std::list<std::string> msgs_;
    JsonParseHelper helper;
};
}

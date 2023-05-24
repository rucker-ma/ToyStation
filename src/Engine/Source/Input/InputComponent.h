//
// Created by ma on 2023/5/23.
//
#pragma once
#include <functional>

#include "Base/Vector.h"
#include "File/FileUtil.h"
#include "Framework/TComponent.h"
namespace toystation{
enum EventType{
    Down,
    Up,
    Move
};
enum MouseKey{
    LEFT,
    MIDDLE,
    RIGHT
};
enum KeyBoardKey{
    KEY_A = 65,
    KEY_D = 68,
    KEY_E = 69,
    KEY_S = 83,
    KEY_Q = 81,
    KEY_W = 87
};
struct MouseEvent{
    EventType type;
    MouseKey key;
    Vector2 position;
    Vector2 delta;
};
struct KeyboardEvent{
    EventType type;
    KeyBoardKey key;
    int repeat;
};
struct CustomInputEvent{
    std::string name;
    Json::Value packet;
};

struct CustomInputEventResponse{
    std::string name;
    std::function<void(Json::Value packet)> handler;
};

class InputComponent :public TComponent{
public:
    InputComponent()=default;
    virtual ComponentType GetType(){return Component_Input;}
    virtual ~InputComponent()=default;
    virtual void OnMouseEvent(MouseEvent event) =0;
    virtual void OnKeyboardEvent(KeyboardEvent event) =0;
};
}

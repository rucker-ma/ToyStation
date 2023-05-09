//
// Created by ma on 2023/3/5.
//
#include "InputSystem.h"
#include <cassert>


#include "ToyEngine.h"

namespace toystation{

std::map<std::string ,InputAction> kActionName{
    {"keydown",InputAction::PRESS},
    {"keyup",InputAction::RELEASE},
    {"dragmove",InputAction::DRAG_MOVE},
    {"lockmove",InputAction::LOCK_MOVE}
};
void InputSystem::Initialize() {
}
void InputSystem::Tick() {
    while(!msgs_.empty()) {
        std::string buffer = msgs_.front();
        msgs_.pop_front();

        Json::Value msg;
        if (helper.parse(buffer.data(), buffer.size(), msg)) {
            auto iter = kActionName.find(msg["type"].asString());
            if(iter!=kActionName.end()){
                switch(iter->second){
                    case InputAction::PRESS:
                    case InputAction::RELEASE:
                        OnKey(iter->second,msg["value"].asInt());
                        break ;
                    case InputAction::DRAG_MOVE:
                        drag_mvoe_.push_back({ msg["value"]["deltax"].asInt(),
                                                msg["value"]["deltay"].asInt()});
                        break ;
                    case InputAction::LOCK_MOVE:
                        lock_move_.push_back({ msg["value"]["deltax"].asInt(),
                                              msg["value"]["deltay"].asInt()});
                        break ;
                }
            }else{
                std::string type_str = msg["type"].asString();
                if(type_str == std::string("capture")){
                    kEngine.PushRenderFlag(RenderAction::Render_RenderDocCapture);
                }else if(type_str == std::string("shader")){
                    //重新编译shader,成功才更新pipeline
                    if(kEngine.GetShaderSystem().ReloadShader()) {
                        kEngine.PushRenderFlag(
                            RenderAction::Render_UpdatePipeline);
                    }
                }
            }
        }else{
            LogError("parse input message error.");
        }
    }
}
void InputSystem::OnKey(InputAction action,int keycode) {
//    std::lock_guard<std::mutex> lck(mtx);
    auto iter= std::find(pressed_keys_.begin(),
                              pressed_keys_.end(),keycode);
    switch (action) {
        case PRESS:
            if(iter == pressed_keys_.end()){
                pressed_keys_.push_back(keycode);
                pressed_counter_[keycode] = 0;
            }
            pressed_counter_[keycode]++;
            break ;
        case RELEASE:
            assert(iter!=pressed_keys_.end());
            pressed_keys_.erase(iter);
            break ;
        default:
            break ;
    }
}

std::pair<int,int> InputSystem::GetMoveDelta(InputAction action){
    switch (action) {
        case DRAG_MOVE:
            return GetMove(drag_mvoe_);
        case LOCK_MOVE:
            return GetMove(lock_move_);
        default:
            return {0,0};
    }
}
std::pair<int,int> InputSystem::GetMove(std::list<std::pair<int,int>>& container){
    std::pair<int,int> result = {0,0};
    if(container.empty()){
        return result;
    }
    for (auto& pair:container) {
        result.first += pair.first;
        result.second+=pair.second;
    }
    container.clear();
    return result;
}

int InputSystem::TakeoutOneKey(int keycode) {
//    std::lock_guard<std::mutex> lck(mtx);
    int count = 0;
    if(pressed_counter_.find(keycode) != pressed_counter_.end()){
        count = pressed_counter_[keycode];
        pressed_counter_[keycode] =0;
    }
    return count;
}

void InputSystem::OnInputMessage(std::string msg){
    msgs_.push_back(msg);
}
}
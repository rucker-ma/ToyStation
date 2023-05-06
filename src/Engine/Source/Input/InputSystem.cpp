//
// Created by ma on 2023/3/5.
//
#include "InputSystem.h"
#include <cassert>

#include "File/FileUtil.h"
#include "ToyEngine.h"

namespace toystation{


void InputSystem::Initialize() {
    process_input_future_ = std::async(std::launch::async,std::bind(&InputSystem::ProcessRemoteMessage,this));
}
void InputSystem::Tick() {
    
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
void InputSystem::OnMoveDelta(int x,int y) {
    move_deltas_.push_back({x,y});
}
std::pair<int,int> InputSystem::GetMoveDelta(){
    std::pair<int,int> result = {0,0};
    if(move_deltas_.empty()){
        return result;
    }
    for (auto& pair:move_deltas_) {
        result.first += pair.first;
        result.second+=pair.second;
    }
    move_deltas_.clear();
    return result;
}
void InputSystem::OnFocus(bool enable) {}
bool InputSystem::IsFoucs() { return true; }
bool InputSystem::IsPress(int keycode){
    std::lock_guard<std::mutex> lck(mtx);
    return std::find(pressed_keys_.begin(),pressed_keys_.end(),keycode)
           !=pressed_keys_.end();
}
int InputSystem::PressedKey()
{
    if(pressed_keys_.empty()){
        return KEY_INVALID;
    }
    return pressed_keys_.front();
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

std::map<std::string ,InputAction> kActionName{
    {"keydown",InputAction::PRESS},
    {"keyup",InputAction::RELEASE},
    {"mousemove",InputAction::MOUSEMOVE}
};

void InputSystem::ProcessRemoteMessage(){
    JsonParseHelper helper;

    while (true){
        if(msgs_.empty()) {
            std::mutex wait_mtx;
            std::unique_lock<std::mutex> lck(wait_mtx);
            cv_.wait(lck);
        }else{
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
                        case InputAction::MOUSEMOVE:
                            move_deltas_.push_back({ msg["value"]["deltax"].asInt(),
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
}
void InputSystem::OnInputMessage(std::string msg){
    msgs_.push_back(msg);
    cv_.notify_one();
}
}
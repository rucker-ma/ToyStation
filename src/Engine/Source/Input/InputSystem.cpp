//
// Created by ma on 2023/3/5.
//
#include "InputSystem.h"
#include <cassert>

//#include "ToyEngine.h"

namespace toystation{

std::map<std::string ,EventType> kEventNameType{
    {"down",EventType::Down},
    {"up",EventType::Up},
    {"move",EventType::Move}
};

void InputSystem::Initialize() {
}
void InputSystem::Tick() {
    ProcessCache();

//    std::list<std::string> msgs;
//    {
//        std::unique_lock<std::mutex> lck(mtx_);
//        msgs = msgs_;
//        msgs_.clear();
//    }
//    while(!msgs.empty()) {
//        std::string buffer = msgs.front();
//        msgs.pop_front();
//
//        Json::Value msg;
//        if (helper.parse(buffer.data(), buffer.size(), msg)) {
//            auto iter = kActionName.find(msg["type"].asString());
//            if(iter!=kActionName.end()){
//                switch(iter->second){
//                    case InputAction::PRESS:
//                    case InputAction::RELEASE:
//                        OnKey(iter->second,msg["value"].asInt());
//                        break ;
//                    case InputAction::DRAG_MOVE:
//                        drag_mvoe_.push_back({ msg["value"]["deltax"].asInt(),
//                                                msg["value"]["deltay"].asInt()});
//                        break ;
//                    case InputAction::LOCK_MOVE:
//                        lock_move_.push_back({ msg["value"]["deltax"].asInt(),
//                                              msg["value"]["deltay"].asInt()});
//                        break ;
//                    case InputAction::CLICK:
//
//                        break ;
//                }
//            }else{
//                std::string type_str = msg["type"].asString();
//                if(type_str == std::string("capture")){
//                    kEngine.PushRenderFlag(RenderAction::Render_RenderDocCapture);
//                }else if(type_str == std::string("shader")){
//                    //重新编译shader,成功才更新pipeline
//                    if(kEngine.GetShaderSystem().ReloadShader()) {
//                        kEngine.PushRenderFlag(
//                            RenderAction::Render_UpdatePipeline);
//                    }
//                }
//            }
//        }else{
//            LogError("parse input message error.");
//        }
//    }
}
//如果相邻两次的事件类型是一样的，可以合并在一次里，同时repeat+1
void InputSystem::ProcessCache(){
    std::vector<MouseEvent> mouse_events;
    std::vector<KeyboardEvent> key_events;
    std::vector<CustomInputEvent> event_cache;
    {
        std::unique_lock<std::mutex> lck(mtx_);
        mouse_events = mouse_cache_;
        key_events = key_cache_;
        event_cache = event_cache_;
        mouse_cache_.clear();
        key_cache_.clear();
        event_cache_.clear();
    }
//    std::vector<MouseEvent> combined_mouse_events;
//    std::vector<MouseEvent> combined_key_events;
//    for(auto& event:mouse_events){
//        if(!combined_mouse_events.empty()){
//            //连续的鼠标移动
//
//        }else{
//            combined_key_events.push_back(event);
//        }
//    }
    for(auto controller:controllers_){
        for(auto& event:mouse_events){
            controller->OnMouseEvent(event);
        }
        for(auto& event:key_events){
            controller->OnKeyboardEvent(event);
        }
    }
    for(auto& event:event_cache){
        //执行事件响应
        if(registered_response_.find(event.name)!=registered_response_.end()){
            registered_response_[event.name]->handler(event.packet);
        }
    }
}


//void InputSystem::OnKey(InputAction action,int keycode) {
////    std::lock_guard<std::mutex> lck(mtx);
//    auto iter= std::find(pressed_keys_.begin(),
//                              pressed_keys_.end(),keycode);
//    switch (action) {
//        case PRESS:
//            if(iter == pressed_keys_.end()){
//                pressed_keys_.push_back(keycode);
//                pressed_counter_[keycode] = 0;
//            }
//            pressed_counter_[keycode]++;
//            break ;
//        case RELEASE:
//            assert(iter!=pressed_keys_.end());
//            pressed_keys_.erase(iter);
//            break ;
//        default:
//            break ;
//    }
//}


//std::pair<int,int> InputSystem::GetMoveDelta(InputAction action){
//    switch (action) {
//        case DRAG_MOVE:
//            return GetMove(drag_mvoe_);
//        case LOCK_MOVE:
//            return GetMove(lock_move_);
//        default:
//            return {0,0};
//    }
//}


//std::pair<int,int> InputSystem::GetMove(std::list<std::pair<int,int>>& container){
//    std::pair<int,int> result = {0,0};
//    if(container.empty()){
//        return result;
//    }
//    for (auto& pair:container) {
//        result.first += pair.first;
//        result.second+=pair.second;
//    }
//    container.clear();
//    return result;
//}

//int InputSystem::TakeoutOneKey(int keycode) {
//    int count = 0;
//    if(pressed_counter_.find(keycode) != pressed_counter_.end()){
//        count = pressed_counter_[keycode];
//        pressed_counter_[keycode] =0;
//    }
//    return count;
//}
void InputSystem::RegisterInputHanle(std::shared_ptr<InputComponent> component){
    if(std::find(controllers_.begin(), controllers_.end(), component)
        == controllers_.end()) {
        controllers_.push_back(component);
    }
}
void InputSystem::UnregisterInputHanle(std::shared_ptr<InputComponent> component){
    controllers_.remove(component);
}
void InputSystem::RegisterCustomResponse(std::shared_ptr<CustomInputEventResponse> response){
    registered_response_[response->name] = response;
}
void InputSystem::UnregisterCustomResponse(std::shared_ptr<CustomInputEventResponse> response){
    registered_response_.erase(response->name);
}
void InputSystem::UnregisterCustomResponse(std::string event_name){
    registered_response_.erase(event_name);
}
void InputSystem::OnInputMessage(std::string msg){
    std::unique_lock<std::mutex> lck(mtx_);
    Json::Value json;
    if(!helper.parse(msg.data(), msg.size(), json)){
        return;
    }
    std::string trigger = json["trigger"].asString();
    if(trigger == "mouse"){
       MouseEvent mouse_event;
       mouse_event.type = kEventNameType[json["type"].asString()];
       mouse_event.key = static_cast<MouseKey>(json["key"].asInt());
       mouse_event.position = Vector2(
       json["position"]["x"].asFloat(), json["position"]["y"].asFloat());
       mouse_event.delta = Vector2(
           json["delta"]["x"].asFloat(), json["delta"]["y"].asFloat());

       mouse_cache_.push_back(mouse_event);
    }else if(trigger == "keyboard"){
       KeyboardEvent key_event;
        key_event.type = kEventNameType[json["type"].asString()];
        key_event.key = static_cast<KeyBoardKey>(json["key"].asInt());
        key_event.repeat =1;
        key_cache_.push_back(key_event);
    }else if (trigger == "event"){
        std::string name = json["name"].asString();
        if(registered_response_.find(name)!=registered_response_.end()){
            event_cache_.push_back({name,json["packet"]});
        }
    }
}
}
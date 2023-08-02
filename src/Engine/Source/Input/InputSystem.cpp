//
// Created by ma on 2023/3/5.
//
#include "InputSystem.h"
#include <cassert>

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
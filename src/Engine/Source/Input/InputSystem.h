//
// Created by ma on 2023/3/5.
//
#pragma once
#include <future>
#include <list>
#include <map>
#include <mutex>
#include <string>

#include "File/FileUtil.h"
#include "InputComponent.h"

namespace toystation{

class InputSystem{
public:
    void Initialize();
    void Tick();
    //Tigger function
    //contain keyboard key and mouse key
//    void OnKey(InputAction action,int keycode);
//    std::pair<int,int> GetMoveDelta(InputAction action);

    //int TakeoutOneKey(int keycode);
    void OnInputMessage(std::string msg);
    void RegisterInputHanle(std::shared_ptr<InputComponent> component);
    void UnregisterInputHanle(std::shared_ptr<InputComponent> component);
    void RegisterCustomResponse(std::shared_ptr<CustomInputEventResponse> response);
    void UnregisterCustomResponse(std::shared_ptr<CustomInputEventResponse> response);
    void UnregisterCustomResponse(std::string event_name);
private:
    void ProcessCache();
private:
    //std::pair<int,int> GetMove(std::list<std::pair<int,int>>& container);
private:
    //TODO:record key press time
    std::list<int> pressed_keys_;
    std::map<int,int> pressed_counter_;
    std::list<std::pair<int,int>> drag_mvoe_;
    std::list<std::pair<int,int>> lock_move_;
    std::mutex mtx_;
    std::list<std::string> msgs_;
    JsonParseHelper helper;

    std::list<std::shared_ptr<InputComponent>> controllers_;
    std::vector<MouseEvent> mouse_cache_;
    std::vector<KeyboardEvent> key_cache_;
    //当前为一对一，后续考虑使用multimap 一对多
    std::map<std::string,std::shared_ptr<CustomInputEventResponse>> registered_response_;
    std::vector<CustomInputEvent> event_cache_;

};
}

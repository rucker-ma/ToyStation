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
#include "Meta/Meta.h"
namespace toystation{

class InputSystem{
    GENERATE_BODY(InputSystem)
public:
    void Initialize();
    void Tick();
    void OnInputMessage(std::string msg);
    void RegisterInputHanle(std::shared_ptr<InputComponent> component);
    void UnregisterInputHanle(std::shared_ptr<InputComponent> component);
    void RegisterCustomResponse(std::shared_ptr<CustomInputEventResponse> response);
    void UnregisterCustomResponse(std::shared_ptr<CustomInputEventResponse> response);

private:
    void ProcessCache();
private:
    //TODO:record key press time
    std::list<int> pressed_keys_;
    std::map<int,int> pressed_counter_;
    std::list<std::pair<int,int>> drag_mvoe_;
    std::list<std::pair<int,int>> lock_move_;

    std::list<std::string> msgs_;
    SKIP_GENERATE(
    JsonParseHelper helper;
    std::mutex mtx_;
    )
    std::list<std::shared_ptr<InputComponent>> controllers_;
    std::vector<MouseEvent> mouse_cache_;
    std::vector<KeyboardEvent> key_cache_;
    //当前为一对一，后续考虑使用multimap 一对多
    std::map<std::string,std::shared_ptr<CustomInputEventResponse>> registered_response_;
    std::vector<CustomInputEvent> event_cache_;

};
}

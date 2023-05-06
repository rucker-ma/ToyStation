//
// Created by ma on 2023/3/17.
//
#pragma once

#include <list>
#include <map>
#include <memory>
#include <string>
#include <atomic>

#include "TComponent.h"

namespace toystation {

class TObject {
public:
    TObject()=default;
    TObject(std::string name);
    const int GetID()const{return id_;}
    template <class T,typename = std::enable_if_t<std::is_base_of_v<TComponent, T>>>
    std::shared_ptr<T> CreateComponent() {
        auto component = std::make_shared<T>();
        AddComponent(component);
        return component;
    }

    //获取组件，如果不存在返回nullptr
    template <class T,typename = std::enable_if_t<std::is_base_of_v<TComponent, T>>>
    std::shared_ptr<T> GetComponent() {
        if (components_.find(T::Type)!=components_.end()){
            return std::dynamic_pointer_cast<T>(components_[T::Type]);
        }
        return nullptr;
    }
    //    template <class T,typename = std::enable_if_t<std::is_base_of_v<TComponent, T>>>
//    std::shared_ptr<T> GetComponent(ComponentType type) {
//        if (components_.find(type)!=components_.end()){
//            return dynamic_cast<std::shared_ptr<T>>(components_[type]);
//        }
//        return nullptr;
//    }
    template <class T,typename = std::enable_if_t<std::is_base_of_v<TComponent, T>> >
    void AddComponent(std::shared_ptr<T> component ){
        components_.insert(std::make_pair(component->GetType(),component));
    }
    template <class T,typename = std::enable_if_t<std::is_base_of_v<TComponent, T>> >
    void RemoveComponent(std::shared_ptr<T>component){
        components_.erase(component->GetType());
    }
    virtual void Tick(){}
private:
    static std::atomic<int> kIDAllocator;

    std::map<ComponentType, std::shared_ptr<TComponent>> components_;
    std::string name_;
    const int id_ = ++kIDAllocator;
};

}  // namespace toystation

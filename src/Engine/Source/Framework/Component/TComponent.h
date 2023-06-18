//
// Created by ma on 2023/3/18.
//

#pragma once
#include <memory>

namespace toystation{
enum ComponentType{
    Component_Mesh,
    Component_Material,
    Component_Camera,
    Component_Input,
    Component_Text
};
class TObject;
class TComponent
{
public:
    virtual ~TComponent()=default;
    virtual ComponentType GetType()=0;
    void SetParent( std::shared_ptr<TObject> obj){parent_ = obj;}
    std::shared_ptr<TObject> Parent(){ return parent_;}
private:
    std::shared_ptr<TObject> parent_;
};
}
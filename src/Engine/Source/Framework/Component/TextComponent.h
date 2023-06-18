//
// Created by ma on 2023/6/4.
//
#pragma once
#include <string>
#include "TComponent.h"
#include "Base/Vector.h"

namespace toystation {
class TextComponent : public TComponent {
public:
    const static ComponentType Type = ComponentType::Component_Text;
    ComponentType GetType() override{return Type;}
    void SetText(std::string text){text_ = text;}
    void SetLocation(Vector2 loc){loc_ = loc;}
    void SetColor(Vector3 color){color_ = color;}
    std::string Text(){return text_;}
    Vector2 Location(){return loc_;}
private:
    std::string text_;
    Vector2 loc_;
    Vector3 color_;
};
}
//
// Created by ma on 2023/3/18.
//

#pragma once

namespace toystation{
enum ComponentType{
    Component_Mesh,
    Component_Material,
    Component_Camera
};

class TComponent
{
public:
    virtual ~TComponent()=default;
    virtual ComponentType GetType()=0;
};
}
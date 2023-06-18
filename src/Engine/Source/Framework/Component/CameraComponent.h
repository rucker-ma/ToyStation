//
// Created by ma on 2023/3/18.
//

#pragma once
#include "Base/Vector.h"
#include "TComponent.h"

namespace toystation{
//          world space
//           z
//           |
//           |
//           |
//           |
//           ----------------> y
//          /
//         /
//        /
//       /
//      x

class CameraComponent:public TComponent{
public:
    const static ComponentType Type = ComponentType::Component_Camera;
    CameraComponent();
    virtual ComponentType GetType();
    Matrix4 GetView();
    Matrix4 GetProjection();
    Vector3& GetPosition();
    void SetAspect(float aspect);
    void Rotate(int x,int y);
    void ViewMove(float x,float y);
    void Move(Vector3 f);
private:
    void Update();
private:
    Vector3 position_;
    Vector3 up_;
    Vector3 forward_;

    Vector3 view_up_;
    Vector3 view_right_;

    float aspect_{1.77};
    float yaw_;
    float pitch_;
    float fovy_{60};
};
}

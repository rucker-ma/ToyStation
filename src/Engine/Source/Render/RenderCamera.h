#pragma once

#include "Base/Vector.h"

namespace toystation {
//          world space
//           y
//           |
//           |
//           |
//           |
//           ----------------> x
//          /
//         /
//        /
//       /
//      z

class RenderCamera {
public:
    // static const Vector3 X;
    // static const Vector3 Y;
    // static const Vector3 Z;


    RenderCamera();
    Matrix4 GetView();
    Matrix4 GetProjection();
    Vector3 GetPosition();
    void SetAspect(float aspect);

private:
    void Update();
private:

    Vector3 position_;

    Vector3 up_;
    Vector3 forward_;
    
    Vector3 view_up_;
    Vector3 view_right_;

    float aspect_{1.77};
    float yaw_{-90};
    float pitch_{0};
    float fovy_{60};
};

// const Vector3 RenderCamera::X = {1.0, 0.0, 0.0};
// const Vector3 RenderCamera::Y = {0.0, 1.0, 0.0};
// const Vector3 RenderCamera::Z = {0.0, 0.0, 1.0};

}  // namespace toystation
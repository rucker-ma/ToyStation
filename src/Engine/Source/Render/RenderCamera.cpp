#include "RenderCamera.h"

namespace toystation {

RenderCamera::RenderCamera() {
    position_ = {0.0, 0.0, 0.0};
    forward_ = {1.0, 0.0, 0.0};
    up_ = {0.0, 0.0, 1.0};
    yaw_ = 0;
    pitch_ = 0;
    view_up_ = up_;
    Update();
}
Matrix4 RenderCamera::GetView() {
//    return glm::lookAt(position_, glm::vec3(0.0F, 0.0F, 0.0F),
//                     glm::vec3(0.0F, 0.0F, 1.0F));
    Update();
    return glm::lookAt(position_, position_ + forward_, view_up_);
}
Matrix4 RenderCamera::GetProjection() {
    Matrix4 projection =
        glm::perspective(glm::radians(fovy_), 1920 / (float)1080, 0.1F, 30.0F);

    projection[1][1] *= -1;
    return projection;
}
Vector3& RenderCamera::GetPosition() { return position_; }
void RenderCamera::SetAspect(float aspect) { aspect_ = aspect; }

void RenderCamera::Update() {
    forward_[0] = cosf(glm::radians(yaw_)) * cosf(glm::radians(pitch_));
    forward_[1] = sinf(glm::radians(pitch_));
    forward_[2] = sinf(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    view_right_ = glm::normalize(glm::cross(forward_, up_));
    view_up_ = glm::normalize(glm::cross(view_right_, forward_));
}
}  // namespace toystation
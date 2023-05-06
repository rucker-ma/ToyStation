//
// Created by ma on 2023/3/18.
//
#include "CameraComponent.h"
namespace toystation{
CameraComponent::CameraComponent(){
    position_ = {-20.0, 0.0, 0.0};
    forward_ = {1.0, 0.0, 0.0};
    up_ = {0.0, 0.0, 1.0};
    yaw_ = 0;
    pitch_ = 0;
    view_up_ = up_;
    Update();
}
ComponentType CameraComponent::GetType(){
    return ComponentType::Component_Camera;
}
Matrix4 CameraComponent::GetView() {
    //    return glm::lookAt(position_, glm::vec3(0.0F, 0.0F, 0.0F),
    //                     glm::vec3(0.0F, 0.0F, 1.0F));
    Update();
    return glm::lookAt(position_, position_ + forward_, view_up_);
}
Matrix4 CameraComponent::GetProjection() {
    Matrix4 projection =
        glm::perspective(glm::radians(fovy_), 1920 / (float)1080, 0.1F, 100.0F);

    projection[1][1] *= -1;
    return projection;
}
Vector3& CameraComponent::GetPosition() { return position_; }
void CameraComponent::SetAspect(float aspect) { aspect_ = aspect; }
void CameraComponent::Rotate(int x,int y){
    Vector4 pos = Vector4 (position_,1.0);
    pos = glm::rotate<float>(Matrix4(1.0),x/100.0,Vector3(0,0,1))*pos;
    pos = pos/pos.w;
    pos = glm::rotate<float>(Matrix4(1.0),y/100.0,Vector3(1,0,0))*pos;
    pos = pos/pos.w;
    position_ = Vector3(pos);
}
void CameraComponent::Update() {
    forward_[0] = cosf(glm::radians(yaw_)) * cosf(glm::radians(pitch_));
    forward_[1] = sinf(glm::radians(pitch_));
    forward_[2] = sinf(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    view_right_ = glm::normalize(glm::cross(forward_, up_));
    view_up_ = glm::normalize(glm::cross(view_right_, forward_));
}
}
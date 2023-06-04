//
// Created by ma on 2023/3/18.
//
#include "CameraComponent.h"
namespace toystation{
CameraComponent::CameraComponent(){
    position_ = {0.0, 5.0, 0.0};
    forward_ = {0.0, 0.0, 1.0};
    up_ = {0.0, 1.0, 0.0};
    yaw_ = 90;
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
        glm::perspective(glm::radians(fovy_), 1920 / (float)1080, 1.0F, 3000.0F);

    //projection[1][1] *= -1;
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
void CameraComponent::ViewMove(float x, float y) {
    yaw_ +=x;
    pitch_ +=y;
    if(pitch_> 89.9f){
        pitch_ = 89.9;
    }
    if(pitch_ < -89.9){
        pitch_ = -89.9;
    }
    Update();
}
void CameraComponent::Move(Vector3 f){
    //相机的移动向前为视角方向，向上为world的向上方式，向右则叉乘，相机高度不应该改变
    Vector3 move_right = glm::normalize(glm::cross({0.0,1.0,0.0},forward_));
    position_ += forward_*f.z;
//    position_ += Vector3 (0.0,1.0,0.0)*f.y;
    position_ +=view_up_*f.y;
    position_ += move_right*f.x;
    Update();
}
void CameraComponent::Update() {
    forward_[0] = cosf(glm::radians(yaw_)) * cosf(glm::radians(pitch_));
    forward_[1] = sinf(glm::radians(pitch_));
    forward_[2] = sinf(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    view_right_ = glm::normalize(glm::cross(up_,forward_));
    view_up_ = glm::normalize(glm::cross(forward_,view_right_));
}
}
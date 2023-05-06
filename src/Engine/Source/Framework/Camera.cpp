#include "Camera.h"
#include "ToyEngine.h"

namespace toystation {

Camera::Camera() : TObject("default camera"){
    camera_component_ = CreateComponent<CameraComponent>();
}
void  Camera::Tick(){
    Vector3& position = camera_component_->GetPosition();
    InputSystem& input = kEngine.GetInputSystem();
    Move(position, Vector3(-0.1, 0, 0),input.TakeoutOneKey(KEY_W));
    Move(position, Vector3(0.1, 0, 0),input.TakeoutOneKey(KEY_S));
    Move(position, Vector3(0.0, -0.1, 0),input.TakeoutOneKey(KEY_A));
    Move(position, Vector3(0.0, 0.1, 0),input.TakeoutOneKey(KEY_D));

    auto move_delta = input.GetMoveDelta();
    if(!(move_delta.first==0 && move_delta.second == 0)){
        camera_component_->Rotate(move_delta.first,move_delta.second);
        LogInfo("camera rotate!!!");
    }
}
void Camera::Move(Vector3& position, Vector3 dir,int length){
    if(length !=0) {
        position += dir * (float)length;
    }
}
}  // namespace toystation
#include "EditorController.h"
#include "ToyEngine.h"

namespace toystation {

EditorController::EditorController() : TObject("default camera"){
    camera_component_ = CreateComponent<CameraComponent>();
    input_component_ = CreateComponent<EditorInputComponent>();
    move_factor_ = 0.3;

    kEngine.GetInputSystem().RegisterInputHanle(input_component_);
}
void EditorController::Init(){
    input_component_->OnScreenHit = [this](Vector2 point){
        ConvertHitToRay(point);
    };
}
void EditorController::ConvertHitToRay(Vector2 screen_point){
    //将屏幕空间转换到世界空间，以便于包围盒求交
    Matrix4  proj_mat = camera_component_->GetProjection();
    Matrix4 view_mat = camera_component_->GetView();
    Matrix4 p_v_inv = glm::inverse(proj_mat*view_mat);
    Vector4  world_near = p_v_inv*Vector4 (screen_point,0,1);
    Vector4 world_far = p_v_inv*Vector4 (screen_point,1,1);
    Ray ray;
    ray.origin = Vector3 (world_near);
    ray.direction = glm::normalize(Vector3 (world_far) - ray.origin);

}
Vector3 EditorController::Location(){
    return camera_component_->GetPosition();
}
void  EditorController::Tick(){

    if(input_component_->view_enable_&& input_component_->view_delta_ !=Vector2(0)){
        camera_component_->ViewMove(-input_component_->view_delta_.x*0.3*2400,
                                    -input_component_->view_delta_.y*0.3*1600);
        input_component_->view_delta_ = Vector2 (0);
    }
    if(input_component_->move_delta_ !=Vector3(0.0)){
        camera_component_->Move(input_component_->move_delta_*move_factor_);
        input_component_->move_delta_ = Vector3 (0);
    }
}

void EditorController::EditorInputComponent::OnMouseEvent(MouseEvent event){
    switch (event.type) {
        case EventType::Down:
            if(event.key == MouseKey::RIGHT){
                view_enable_ = true;
                view_delta_ = {0,0};
            }
            if (event.key == MouseKey::LEFT) {
                //convert screen point to hit ray
                OnScreenHit(event.position);
            }
            break ;
        case EventType::Move:
            if(event.key == MouseKey::RIGHT && view_enable_){
                view_delta_ +=event.delta;
            }
            break ;
        case EventType::Up:
            if(event.key == MouseKey::RIGHT){
                view_enable_ = false;
            }
            break ;
    }
}
void EditorController::EditorInputComponent::OnKeyboardEvent(KeyboardEvent event){
    switch (event.type) {
        case EventType::Down:
            ComputeMove(event);
            break ;
        case EventType::Up:
            break ;
    }
}
void EditorController::EditorInputComponent::ComputeMove(KeyboardEvent event){
    switch (event.key) {
            //w +z,s -z
        case KEY_S:
            move_delta_.z -=event.repeat;
            break ;
        case KEY_W:
            move_delta_.z +=event.repeat;
            break ;
            //d +x,a -x
        case KEY_A:
            move_delta_.x -=event.repeat;
            break ;
        case KEY_D:
            move_delta_.x +=event.repeat;
            break ;
            //e +y,q -y
        case KEY_E:
            move_delta_.y +=event.repeat;
            break ;
        case KEY_Q:
            move_delta_.y -=event.repeat;
            break ;
    }
}
}  // namespace toystation
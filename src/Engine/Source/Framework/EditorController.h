#pragma once

#include "Base/Vector.h"
#include "CameraComponent.h"
#include "Input/InputComponent.h"
#include "TObject.h"
namespace toystation {

class EditorController: public TObject {

    class EditorInputComponent:public InputComponent{
    public:
        virtual void OnMouseEvent(MouseEvent event);
        virtual void OnKeyboardEvent(KeyboardEvent event);
    private:
        void ComputeMove(KeyboardEvent event);
    private:
        std::function<void(Vector2 point)> OnScreenHit;
        bool view_enable_;
        Vector2 view_delta_;
        Vector3 move_delta_;
        friend class EditorController;
    };

public:
    EditorController();
    void Init();
    virtual void  Tick();
    Vector3 Location();
private:
    void ConvertHitToRay(Vector2 screen_point);
private:
    std::shared_ptr<CameraComponent> camera_component_;
    std::shared_ptr<EditorInputComponent> input_component_;
    float move_factor_;
};


}  // namespace toystation
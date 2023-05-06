#pragma once

#include "Base/Vector.h"
#include "TObject.h"
#include "CameraComponent.h"
namespace toystation {

class Camera: public TObject{
public:
    Camera();
    virtual void  Tick();
private:
    void Move(Vector3& position, Vector3 dir,int length);
private:
    std::shared_ptr<CameraComponent> camera_component_;
};


}  // namespace toystation
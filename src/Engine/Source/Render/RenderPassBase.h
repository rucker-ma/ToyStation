#pragma once
namespace toystation {
class RenderPassBase {
public:
    virtual void Initialize(){};
    virtual void PostInitialize(){};
    virtual void PrepareData(){};
};
}  // namespace toystation
#pragma once

#include "RenderPassBase.h"

namespace toystation {

class TextPass : public RenderPassBase {
public:
    virtual ~TextPass() = default;
    virtual void Initialize(RenderPassInitInfo& info) override;
    virtual void Draw() override;
};
}  // namespace toystation
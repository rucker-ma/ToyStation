//
// Created by ma on 2023/3/10.
//
#pragma once

namespace toystation {
class RenderDocCapture {
public:
    static RenderDocCapture& Instance();
    bool Init();
    void StartCapture();
    void TriggerCapture();
    void EndCapture();

private:
    RenderDocCapture() = default;

private:
    // RENDERDOC_API_1_1_2 *
    void* rdoc_api_ = nullptr;
    unsigned int rdoc_pid_ =0;
};
}  // namespace toystation

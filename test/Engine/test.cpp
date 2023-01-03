#include <gtest/gtest.h>

#include "Render/RenderSystem.h"

using namespace toystation;

RenderSystem render_system;

TEST(Engine, RenderInit) {
    render_system.Initialize();
}

TEST(Engine, RenderTick) {
    render_system.Tick();
}

int main(int argc, char* argv[]) {
    testing::InitGoogleTest();
    RUN_ALL_TESTS();
    return 0;
}
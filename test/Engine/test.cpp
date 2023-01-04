#include <gtest/gtest.h>

#include "Render/RenderSystem.h"
#include "Transfer/TransferSystem.h"

using namespace toystation;

RenderSystem render_system;
TransferSystem transfer_system;

TEST(Engine, RenderInit) { render_system.Initialize(); }

TEST(Engine, RenderTick) { render_system.Tick(); }

TEST(Engine, TransferInit) { 
    transfer_system.Initialize(); 
}
int main(int argc, char* argv[]) {
    testing::InitGoogleTest();
    RUN_ALL_TESTS();
    return 0;
}
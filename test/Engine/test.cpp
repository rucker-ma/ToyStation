#include <gtest/gtest.h>
#include "Render/VulkanContext_c.h"
#include "TSEngine_c.h"


TEST(Engine,EngineInit){
    VulkanContext_CreateInstance(nullptr,false);
    TEngine_Init();    
}


void main(){
    RUN_ALL_TESTS();
}
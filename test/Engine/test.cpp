#include <gtest/gtest.h>
#include "TSEngine_c.h"


TEST(Engine,EngineInit){
    TEngine_Init();    
}


void main(){
    RUN_ALL_TESTS();
}
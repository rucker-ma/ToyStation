cmake_minimum_required(VERSION 3.19)
project(ToyStation)
set(CMAKE_CXX_STANDARD 17)


set(ASTYLE "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/AStyle")
set(GLM "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/glm-0.9.9.8")
set(LLVM "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/LLVM")
set(STB_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/stb_image")
set(VULKAN "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/Vulkan")

find_library(ASTYLE_LIB NAMES AStyleLibd PATHS ${ASTYLE}/lib)
find_library(LIBCLANG_LIB NAMES libclang PATHS ${LLVM}/lib)
find_library(VULKAN_LIB NAMES vulkan-1 PATHS ${VULKAN}/Lib)

add_subdirectory(./src/Parser)
add_subdirectory(./src/Engine)

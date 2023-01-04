cmake_minimum_required(VERSION 3.19)
project(ToyStation)
set(CMAKE_CXX_STANDARD 17)


set(ASTYLE "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/AStyle")
set(GLM "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/glm-0.9.9.8")
set(LLVM "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/LLVM")
set(STB_IMAGE "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/stb_image")
set(VULKAN "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/Vulkan")
set(SPDLOG "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/spdlog")
set(VK_EXTENSION "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/vk_extension")
set(GTEST "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/googletest")
set(WebRTC "${CMAKE_CURRENT_SOURCE_DIR}/src/ThirdParty/webrtc-m94")


find_library(ASTYLE_LIB NAMES AStyleLibd PATHS ${ASTYLE}/lib)
find_library(LIBCLANG_LIB NAMES libclang PATHS ${LLVM}/lib)
find_library(VULKAN_LIB NAMES vulkan-1 PATHS ${VULKAN}/Lib)
find_library(GTEST_LIB NAMES gtestd PATHS ${GTEST}/lib)
find_library(WebRTC_LIB NAMES webrtc PATHS ${WebRTC}/lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src/Engine/Output)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/src/Engine/Output)
# 设置输出目录要Editor bin目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/src/Editor/Editor/bin/Debug/net6.0)
add_subdirectory(./src/ThirdParty/vk_extension)
add_subdirectory(./src/Parser)
add_subdirectory(./src/Engine)
add_subdirectory(./test/Engine)
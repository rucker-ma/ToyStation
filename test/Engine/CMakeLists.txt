set(MODULE_NAME engine_test)

include_directories(
    ${GTEST}/include
    ${CMAKE_SOURCE_DIR}/src/Engine/Generated
    ${CMAKE_SOURCE_DIR}/src/Engine/Source
    ${GLM}
    ${VULKAN}/Include
    ${SPDLOG}/include

    ${WebRTC}/include
    ${WebRTC}/include/third_party/abseil-cpp

    ${VK_EXTENSION}
)
file(GLOB_RECURSE SOURCE_FILE *.cpp *.h)

add_definitions(-D_HAS_ITERATOR_DEBUGGING=0  -DNOMINMAX -DWEBRTC_WIN -DWIN32_LEAN_AND_MEAN)
add_executable(${MODULE_NAME} ${SOURCE_FILE})

# Disable some link warning
if(MSVC)
    set_target_properties(${MODULE_NAME} PROPERTIES LINK_FLAGS "/ignore:4819")
    
    set_property(TARGET ${MODULE_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
endif()

target_link_libraries(${MODULE_NAME} ${GTEST_LIB} Engine)

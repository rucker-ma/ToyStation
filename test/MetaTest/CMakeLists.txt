set(MODULE_NAME MetaTest)
set(CMAKE_CXX_STANDARD 20)
include_directories(
        ${GTEST}/include
        ${CMAKE_SOURCE_DIR}/src/Engine/Source/Meta
        ${CMAKE_CURRENT_SOURCE_DIR}
)
file(GLOB_RECURSE SOURCE_FILE *.cpp *.h  ${CMAKE_SOURCE_DIR}/src/Engine/Source/Meta/*.cpp)

add_definitions(-D_HAS_ITERATOR_DEBUGGING=0  -DNOMINMAX -DWEBRTC_WIN -DWIN32_LEAN_AND_MEAN)
add_executable(${MODULE_NAME} ${SOURCE_FILE})
target_link_libraries(${MODULE_NAME} ${GTEST_LIB})
# Disable some link warning
#if(MSVC)
#    set_target_properties(${MODULE_NAME} PROPERTIES LINK_FLAGS "/ignore:4819")
#
    set_property(TARGET ${MODULE_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
#endif()

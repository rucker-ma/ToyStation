set(MODULE_NAME meta_test)

include_directories(
        ${GTEST}/include
        ${CMAKE_SOURCE_DIR}/src/Engine/Source/Meta
)
file(GLOB_RECURSE SOURCE_FILE *.cpp *.h)

add_definitions(-D_HAS_ITERATOR_DEBUGGING=0  -DNOMINMAX -DWEBRTC_WIN -DWIN32_LEAN_AND_MEAN)
add_executable(${MODULE_NAME} ${SOURCE_FILE})

# Disable some link warning
#if(MSVC)
#    set_target_properties(${MODULE_NAME} PROPERTIES LINK_FLAGS "/ignore:4819")
#
#    set_property(TARGET ${MODULE_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
#endif()

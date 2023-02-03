set(MODULE_NAME Engine)

find_package(FFmpeg COMPONENTS avcodec avdevice avformat avutil)

include_directories(
    ${GLM}
    ${STB_IMAGE}
    ${VULKAN}/Include
    ${SPDLOG}/include
    ${VK_EXTENSION}
    ${WebRTC}/include
    ${WebRTC}/include/third_party/abseil-cpp
    ${WebRTC}/include/third_party/jsoncpp/source/include
    ${ASIO}/include
    
    ${FFMPEG_INCLUDE_DIR}

    ${WEBSOCKETPP}
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

file(GLOB_RECURSE SOURCE_FILE *.cpp)
file(GLOB_RECURSE HEADER_FILE *.H)

# add_library(${MODULE_NAME} STATIC ${SOURCE_FILE} ${HEADER_FILE})
add_executable(${MODULE_NAME} ${SOURCE_FILE} ${HEADER_FILE})
if(MSVC)
    # disable warning for chinese
    set_target_properties(${MODULE_NAME} PROPERTIES COMPILE_FLAGS "/wd4819")
    set_target_properties(${MODULE_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
    set_property(TARGET ${MODULE_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug")
    add_definitions(-DWEBRTC_WIN -DNOMINMAX -D_HAS_ITERATOR_DEBUGGING=0 -DWEBRTC_USE_H264)
endif()

add_definitions(-DTS_CPP_EXPORT)

# set_target_properties(${MODULE_NAME}
# PROPERTIES
# ARCHIVE_OUTPUT_NAME_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/Output
# LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Output
# RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Output)


target_link_libraries(${MODULE_NAME} ${VULKAN_LIB} VK_EXTENSION)
target_link_libraries(${MODULE_NAME} ${WebRTC_LIB})
target_link_libraries(${MODULE_NAME} ${FFMPEG_LIB})
target_link_libraries(${MODULE_NAME} ${JSONCPP_LIB})
target_link_libraries(${MODULE_NAME} ${FFMPEG_LIBRARIES})
# get_property(DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

# add_custom_command(TARGET ${MODULE_NAME} PRE_BUILD
# DEPENDS Parser
# COMMAND $<TARGET_FILE:Parser> "-I" ${DIRS} "-P" ${CMAKE_CURRENT_SOURCE_DIR}/Source
# "-O" ${CMAKE_CURRENT_SOURCE_DIR}/Generated "-CS" ${CMAKE_CURRENT_SOURCE_DIR}/../Editor/ToyStation.Runtime)

# set runtime output directory at root cmake file, this is not needed
# add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
# COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Engine>
# ${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Editor/bin/$<CONFIG>/net6.0)
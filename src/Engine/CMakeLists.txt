set(MODULE_NAME Engine)
#当前使用的webrtc库不支持c++17标准，因此不能开启c++17
#set(CMAKE_CXX_STANDARD 17)
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
    ${FREETYPE}/include
    ${TINY_GLTF}
    ${RENDERDOC}
    ${NVCodec}/Interface
    ${KTX}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)
#查找cuda库
find_package(CUDAToolkit)
IF(${CUDAToolkit_FOUND})
    add_definitions(-DTOYSTATION_CUDA)
    include_directories(${CUDAToolkit_INCLUDE_DIRS})
ENDIF(${CUDAToolkit_FOUND})


file(GLOB_RECURSE SOURCE_FILE *.cpp)
file(GLOB_RECURSE HEADER_FILE *.H)

# add_library(${MODULE_NAME} STATIC ${SOURCE_FILE} ${HEADER_FILE})
add_executable(${MODULE_NAME} ${SOURCE_FILE} ${HEADER_FILE})
if(MSVC)
    # disable warning for chinese
    set_target_properties(${MODULE_NAME} PROPERTIES COMPILE_FLAGS "/wd4819")
    set_target_properties(${MODULE_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
#    set_property(TARGET ${MODULE_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebugDLL")
    set_property(TARGET ${MODULE_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    add_definitions(-DWEBRTC_WIN -DNOMINMAX -D_HAS_ITERATOR_DEBUGGING=0 -DWEBRTC_USE_H264)
endif()

# PROPERTIES
# ARCHIVE_OUTPUT_NAME_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/Output
# LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Output
# RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Output)

#message("Debug:find vulkan lib: ${VULKAN_LIB}")
target_link_libraries(${MODULE_NAME} ${VULKAN_LIB} ${SHADERC_LIB} VK_EXTENSION)
target_link_libraries(${MODULE_NAME} ${WebRTC_LIB})
target_link_libraries(${MODULE_NAME} ${FFMPEG_LIB})
target_link_libraries(${MODULE_NAME} ${JSONCPP_LIB})
target_link_libraries(${MODULE_NAME} ${FFMPEG_LIBRARIES})
target_link_libraries(${MODULE_NAME} ${FREETYPE_LIB})
target_link_libraries(${MODULE_NAME} ${KTX_LIB})
IF(${CUDAToolkit_FOUND})
    target_link_libraries(${MODULE_NAME} CUDA::cudart)
    target_link_libraries(${MODULE_NAME} CUDA::cuda_driver)
    target_link_libraries(${MODULE_NAME} ${NVCodec_LIB})
ENDIF(${CUDAToolkit_FOUND})
# get_property(DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

# add_custom_command(TARGET ${MODULE_NAME} PRE_BUILD
# DEPENDS Parser
# COMMAND $<TARGET_FILE:Parser> "-I" ${DIRS} "-P" ${CMAKE_CURRENT_SOURCE_DIR}/Source
# "-O" ${CMAKE_CURRENT_SOURCE_DIR}/Generated "-CS" ${CMAKE_CURRENT_SOURCE_DIR}/../Editor/ToyStation.Runtime)

# set runtime output directory at root cmake file, this is not needed
# add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
# COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Engine>
# ${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Editor/bin/$<CONFIG>/net6.0)
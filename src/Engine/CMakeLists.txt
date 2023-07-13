set(MODULE_NAME Engine)

set(CMAKE_CXX_STANDARD 20)
find_package(FFmpeg COMPONENTS avcodec avdevice avformat avutil)
include_directories(
    ${GLM}
    ${STB_IMAGE}
    ${VULKAN}/Include
    ${SPDLOG}/include
    ${VK_EXTENSION}
    ${WebRTC110_PATH}
    ${WebRTC110_PATH}/third_party/abseil-cpp
    ${WebRTC110_PATH}/third_party/jsoncpp/source/include
    ${ASIO}/include
    
    ${FFMPEG_INCLUDE_DIR}

    ${WEBSOCKETPP}
    ${FREETYPE}/include
    ${TINY_GLTF}
    ${RENDERDOC}
    ${NVCodec}/Interface
    ${KTX}/include
    ${NODEJS}/include
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

#根据文件路径生成vs工程中的目录结构
function(set_group_for_ide rootdir filepath)
    string(REPLACE  ${rootdir} "" substr_file ${filepath})
    string(FIND ${substr_file} "/" postion REVERSE)
    string(SUBSTRING  ${substr_file} 0 ${postion} group)
    source_group("${group}" REGULAR_EXPRESSION "${rootdir}${group}/[^/.]+.(cpp|h|c)")
endfunction()

if(MSVC)
    # disable warning for chinese
    set_target_properties(${MODULE_NAME} PROPERTIES COMPILE_FLAGS "/wd4819")
    set_target_properties(${MODULE_NAME} PROPERTIES LINK_FLAGS "/ignore:4099")
    set_property(TARGET ${MODULE_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDLL")
    add_definitions(-DNDEBUG -DWEBRTC_WIN -DNOMINMAX -D_HAS_ITERATOR_DEBUGGING=0 -DWEBRTC_USE_H264)

    foreach(cppfile ${SOURCE_FILE})
        set_group_for_ide(${CMAKE_CURRENT_SOURCE_DIR} ${cppfile})
    endforeach(cppfile)
    foreach(hfile ${HEADER_FILE})
        set_group_for_ide(${CMAKE_CURRENT_SOURCE_DIR} ${hfile})
    endforeach(hfile)
endif()
#win平台上获取的compiler id 无法匹配clang-cl
#if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    # /EHa:开启c++ exceptions

    set_target_properties(${MODULE_NAME} PROPERTIES COMPILE_FLAGS "/EHa")
    add_definitions(-D_SILENCE_CXX20_IS_POD_DEPRECATION_WARNING
                    -D_SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING)
#endif()

#string(COMPARE EQUAL ${CMAKE_BUILD_TYPE} "Debug" IS_DEBUG)
#message("build type: ${IS_DEBUG}")
#if(IS_DEBUG)
#    message("use debug lib")
#    find_library(WebRTC110_LIB NAMES webrtc PATHS ${WebRTC110_PATH}/out/Debug/obj)
#else()
find_library(WebRTC110_LIB NAMES webrtc PATHS ${WebRTC110_PATH}/out/Release/obj)
#endif()
message("webrtc lib path: ${WebRTC110_LIB}")
target_link_libraries(${MODULE_NAME} ${VULKAN_LIB} ${SHADERC_LIB} VK_EXTENSION)
target_link_libraries(${MODULE_NAME} ${WebRTC110_LIB})

target_link_libraries(${MODULE_NAME} ${FFMPEG_LIB})
target_link_libraries(${MODULE_NAME} ${FFMPEG_LIBRARIES})
target_link_libraries(${MODULE_NAME} ${FREETYPE_LIB})
target_link_libraries(${MODULE_NAME} ${KTX_LIB})
target_link_libraries(${MODULE_NAME} ${NODE_LIB})
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
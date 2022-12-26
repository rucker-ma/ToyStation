set(MODULE_NAME VK_EXTENSION)

include_directories(
    ${VULKAN}/Include
)

add_definitions(-DVK_KHR_video_queue -DVK_ENABLE_BETA_EXTENSIONS)

file(GLOB_RECURSE SOURCE_FILE *.cpp *.hpp *.h)
add_library(${MODULE_NAME} STATIC ${SOURCE_FILE})

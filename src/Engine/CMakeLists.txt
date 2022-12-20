set(MODULE_NAME Engine)
include_directories(
    ${GLM}
    ${STB_IMAGE}
    ${VULKAN}/Include
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

file(GLOB_RECURSE SOURCE_FILE *.cpp)
file(GLOB_RECURSE HEADER_FILE *.H)
add_library(${MODULE_NAME} SHARED ${SOURCE_FILE} ${HEADER_FILE})
target_link_libraries(${MODULE_NAME} ${VULKAN_LIB})

get_property(DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

add_custom_command(TARGET ${MODULE_NAME} PRE_BUILD
    DEPENDS Parser
    COMMAND $<TARGET_FILE:Parser> "-I" ${DIRS} "-P" ${CMAKE_CURRENT_SOURCE_DIR}/Source 
    "-O" ${CMAKE_CURRENT_SOURCE_DIR}/Generated "-CS" ${CMAKE_CURRENT_SOURCE_DIR}/../Editor/ToyStation.Runtime)

# add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Engine>
#     ${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Editor/bin/$<CONFIG>/net6.0)
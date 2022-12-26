set(MODULE_NAME Engine)


include_directories(
    ${GLM}
    ${STB_IMAGE}
    ${VULKAN}/Include
    ${SPDLOG}/include
    ${VK_EXTENSION}
    ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

file(GLOB_RECURSE SOURCE_FILE *.cpp)
file(GLOB_RECURSE HEADER_FILE *.H)
add_library(${MODULE_NAME} SHARED ${SOURCE_FILE} ${HEADER_FILE})

# set_target_properties(${MODULE_NAME}
#     PROPERTIES 
#     ARCHIVE_OUTPUT_NAME_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/Output
#     LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Output
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Output)
    

target_link_libraries(${MODULE_NAME} ${VULKAN_LIB} VK_EXTENSION)

get_property(DIRS DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)

# add_custom_command(TARGET ${MODULE_NAME} PRE_BUILD
#     DEPENDS Parser
#     COMMAND $<TARGET_FILE:Parser> "-I" ${DIRS} "-P" ${CMAKE_CURRENT_SOURCE_DIR}/Source 
#     "-O" ${CMAKE_CURRENT_SOURCE_DIR}/Generated "-CS" ${CMAKE_CURRENT_SOURCE_DIR}/../Editor/ToyStation.Runtime)

#set runtime output directory at root cmake file, this is not needed
# add_custom_command(TARGET ${MODULE_NAME} POST_BUILD
#     COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:Engine>
#     ${CMAKE_CURRENT_SOURCE_DIR}/../Editor/Editor/bin/$<CONFIG>/net6.0)
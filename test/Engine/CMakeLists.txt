set(MODULE_NAME engine_test)

include_directories(
    ${GTEST}/include  
    ${CMAKE_SOURCE_DIR}/src/Engine/Generated
    ${CMAKE_SOURCE_DIR}/src/Engine/Source

    ${GLM}
    ${VULKAN}/Include
    ${SPDLOG}/include
    ${VK_EXTENSION}
)
file(GLOB_RECURSE SOURCE_FILE *.cpp *.h)

add_executable(${MODULE_NAME} ${SOURCE_FILE})
target_link_libraries(${MODULE_NAME} ${GTEST_LIB} Engine)
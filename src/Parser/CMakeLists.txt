set(MODULE_NAME Parser)
include_directories(
    ${LLVM}/include
    ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB_RECURSE SOURCE_FILE *.cpp)
file(GLOB_RECURSE HEADER_FILE *.H)
add_executable(${MODULE_NAME} ${SOURCE_FILE} ${HEADER_FILE})

target_link_libraries(${MODULE_NAME} ${ASTYLE_LIB} ${LIBCLANG_LIB})
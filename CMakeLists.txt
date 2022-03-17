cmake_minimum_required(VERSION 3.17)

project(renderer)
set(CMAKE_CXX_STANDARD 17)

find_package(fmt CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

add_custom_target(shaders)

set_source_files_properties(
        include/renderer/shaders/pixel.hlsl
        PROPERTIES
        VS_SHADER_TYPE "ps"
        VS_SHADER_MODEL "5_0"
        VS_SHADER_ENTRYPOINT "ps_main"
        VS_SHADER_VARIABLE_NAME "pixel_shader_data"
)
set_source_files_properties(
        include/renderer/shaders/vertex.hlsl
        PROPERTIES
        VS_SHADER_TYPE "vs"
        VS_SHADER_MODEL "5_0"
        VS_SHADER_ENTRYPOINT "vs_main"
        VS_SHADER_VARIABLE_NAME "vertex_shader_data"
)

function(build_shader SHADER HEADER)
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    get_source_file_property(SHADER_TYPE ${SHADER} VS_SHADER_TYPE)
    get_source_file_property(SHADER_MODEL ${SHADER} VS_SHADER_MODEL)
    get_source_file_property(SHADER_ENTRYPOINT ${SHADER} VS_SHADER_ENTRYPOINT)
    get_source_file_property(SHADER_VARIABLE_NAME ${SHADER} VS_SHADER_VARIABLE_NAME)
    add_custom_command(
            TARGET shaders
            COMMAND fxc.exe /nologo /E${SHADER_ENTRYPOINT} /T${SHADER_TYPE}_${SHADER_MODEL} /Zi /Fo ${CMAKE_BINARY_DIR}/${SHADER_NAME}.cso /Fd ${CMAKE_BINARY_DIR}/${SHADER_NAME}.pdb /Fh ${HEADER} /Vn ${SHADER_VARIABLE_NAME} ${SHADER}
            MAIN_DEPENDENCY ${SHADER}
            BYPRODUCTS ${HEADER}
            COMMENT "HLSL ${SHADER}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            VERBATIM
    )
endfunction()

build_shader(include/renderer/shaders/pixel.hlsl ${CMAKE_CURRENT_SOURCE_DIR}/include/renderer/shaders/compiled/pixel.h)
build_shader(include/renderer/shaders/vertex.hlsl ${CMAKE_CURRENT_SOURCE_DIR}/include/renderer/shaders/compiled/vertex.h)

file(GLOB_RECURSE SOURCES src/*.*)
add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC d3d11 d3dcompiler fmt::fmt glm::glm)

add_dependencies(${PROJECT_NAME} shaders)

add_subdirectory(test)
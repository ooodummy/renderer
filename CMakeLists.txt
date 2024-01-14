cmake_minimum_required(VERSION 3.17)

project(renderer)

set(CMAKE_CXX_STANDARD 23)

add_definitions(-DFMT_HEADER_ONLY)
set(FT_DISABLE_HARFBUZZ ON)

include(FetchContent)

if (NOT TARGET freetype)
    message("Fetching freetype")
    FetchContent_Declare(freetype
            GIT_REPOSITORY https://github.com/freetype/freetype.git
            GIT_TAG        master
    )
    FetchContent_MakeAvailable(freetype)
endif()

if (NOT TARGET glm)
    message("Fetching glm")
    FetchContent_Declare(glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG        master
    )
    FetchContent_MakeAvailable(glm)
endif()

file(MAKE_DIRECTORY include/renderer/shaders/compiled)

message("Building custom shader target")
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
    message("Building shader ${SHADER}")
    get_filename_component(SHADER_NAME ${SHADER} NAME_WE)
    get_source_file_property(SHADER_TYPE ${SHADER} VS_SHADER_TYPE)
    get_source_file_property(SHADER_MODEL ${SHADER} VS_SHADER_MODEL)
    get_source_file_property(SHADER_ENTRYPOINT ${SHADER} VS_SHADER_ENTRYPOINT)
    get_source_file_property(SHADER_VARIABLE_NAME ${SHADER} VS_SHADER_VARIABLE_NAME)
    add_custom_command(
            TARGET shaders PRE_BUILD
            COMMAND fxc.exe /nologo /E${SHADER_ENTRYPOINT} /T${SHADER_TYPE}_${SHADER_MODEL} /Zi /Fo ${CMAKE_BINARY_DIR}/${SHADER_NAME}.cso /Fd ${CMAKE_BINARY_DIR}/${SHADER_NAME}.pdb /Fh ${HEADER} /Vn ${SHADER_VARIABLE_NAME} ${SHADER}
            MAIN_DEPENDENCY ${SHADER}
            BYPRODUCTS ${HEADER}
            COMMENT "HLSL ${SHADER}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            VERBATIM
    )
endfunction()

build_shader(include/renderer/shaders/pixel.hlsl include/renderer/shaders/compiled/pixel.h)
build_shader(include/renderer/shaders/vertex.hlsl include/renderer/shaders/compiled/vertex.h)

file(GLOB_RECURSE SOURCES src/*.*)

message("Building library ${PROJECT_NAME}")
add_library(${PROJECT_NAME} STATIC ${SOURCES} include/renderer/shaders/pixel.hlsl)

message("Including directories")
target_include_directories(${PROJECT_NAME} PUBLIC include)
message("Linking libraries")
target_link_libraries(${PROJECT_NAME} PUBLIC glm::glm freetype)
target_link_libraries(${PROJECT_NAME} PRIVATE d3d11 dxgi dxguid)

add_dependencies(${PROJECT_NAME} shaders)

message("Adding render_test")
add_subdirectory(test renderer_test)
cmake_minimum_required(VERSION 3.17)

project(renderer)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(lib)

file(GLOB_RECURSE SOURCES src/*.*)
add_library(${PROJECT_NAME} STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PUBLIC d3d11 d3dcompiler glm)

add_subdirectory(test)
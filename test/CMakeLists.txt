cmake_minimum_required(VERSION 3.17)

project(test)

file(GLOB_RECURSE SOURCES src/*.*)
add_executable(${PROJECT_NAME} ${SOURCES})

include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PRIVATE carbon renderer dwmapi)
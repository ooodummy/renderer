cmake_minimum_required(VERSION 3.17)

project(test)

add_executable(${PROJECT_NAME} main.cpp)

include_directories(${PROJECT_NAME} PUBLIC include)
target_link_libraries(${PROJECT_NAME} PRIVATE carbon renderer dwmapi)
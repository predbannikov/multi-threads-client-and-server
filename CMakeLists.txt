cmake_minimum_required(VERSION 2.8)
set (CMAKE_CXX_STANDARD 14)
project(mining)
find_package(Threads REQUIRED)
add_executable(${PROJECT_NAME}
    "main.cpp"
    "sha256.h"
    "memory.h"
    "sha256.cpp"
    "test.h"
    "test.cpp"
    )

target_link_libraries(${PROJECT_NAME} Threads::Threads)

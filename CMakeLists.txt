cmake_minimum_required(VERSION 3.20)

project(MiniCell)

set(CMAKE_CXX_STANDARD 20)

add_executable(MiniCell
    Sandbox/main.cpp
    Engine/Core/Logger.cpp
)

target_include_directories(MiniCell PRIVATE
    Engine/Core/
)
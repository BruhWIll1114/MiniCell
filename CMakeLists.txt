cmake_minimum_required(VERSION 3.20)

project(MiniCell)

set(CMAKE_CXX_STANDARD 20)

add_executable(MiniCell
    Sandbox/main.cpp
    Engine/Core/Logger.cpp
    Engine/Assets/AssetManager.cpp
    Engine/Platform/Win32Platform.cpp
)

target_include_directories(MiniCell PRIVATE
    Engine/Core/
    Engine/Containers/
    Engine/Memory/
    Engine/IO/
    Engine/Assets/
    Engine/Platform/
)
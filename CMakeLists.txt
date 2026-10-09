cmake_minimum_required(VERSION 3.20)

project(MiniCell)

set(CMAKE_CXX_STANDARD 20)

add_executable(MiniCell
    Sandbox/main.cpp
    Engine/Core/Logger.cpp
    Engine/Assets/AssetManager.cpp
    Engine/Platform/Win32Platform.cpp
    Engine/IO/FileSystem.cpp
    Engine/Threading/JobSystem.cpp
)

add_custom_command(TARGET MiniCell POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Sandbox/assets
        $<TARGET_FILE_DIR:MiniCell>/assets
)

target_compile_definitions(MiniCell PRIVATE 
    MINICELL_SOURCE_DIR="${CMAKE_SOURCE_DIR}"
)

target_include_directories(MiniCell PRIVATE
    Engine/Core/
    Engine/Containers/
    Engine/Memory/
    Engine/IO/
    Engine/Assets/
    Engine/Platform/
    Engine/Threading/
)
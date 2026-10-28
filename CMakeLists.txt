cmake_minimum_required(VERSION 3.20)

project(OrganOnChip
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

FetchContent_MakeAvailable(json)

add_executable(organ_on_chip
    src/main.cpp
    src/geometry/rectangular_channel.cpp
    src/config_read.cpp
    src/mesh/rectangular_channel_smesh.cpp
    src/mesh/rectangular_channel_vmesh.cpp
)

target_include_directories(organ_on_chip
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(GMSH_ROOT "C:/Users/ccole/Desktop/gmsh-4.15.2-Windows64-sdk/gmsh-4.15.2-Windows64-sdk")

target_include_directories(organ_on_chip PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${GMSH_ROOT}/include"
)

target_link_libraries(organ_on_chip PRIVATE
    "${GMSH_ROOT}/lib/gmsh.dll.lib"
)

target_compile_definitions(organ_on_chip PRIVATE
    PROJ_ROOT="${CMAKE_CURRENT_SOURCE_DIR}"
)

target_link_libraries(organ_on_chip
    PRIVATE
        nlohmann_json::nlohmann_json
)

add_custom_command(TARGET organ_on_chip POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GMSH_ROOT}/lib/gmsh-4.15.dll"
        "$<TARGET_FILE_DIR:organ_on_chip>"
)
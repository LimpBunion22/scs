# Establecer la versión mínima de CMake
cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(SPACE_COMBAT_SIMULATOR)

# Establecer el estándar de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


find_package(nlohmann_json REQUIRED)
find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)
# add_subdirectory(utils/imgui)
add_subdirectory(utils/imgui-sfml)
set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/utils/imgui)
set(IMGUI_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/utils/imgui)

# Incluir todas las subcarpetas de 'include'
include_directories(include)
include_directories(include/astral_bodies)
include_directories(include/general)
include_directories(include/window_handler)
include_directories(include/physics)
include_directories(include/ships)

#files to include
file (GLOB_RECURSE HEADER_FILES "include/*.h"
    "include/astral_bodies/*.h"
    "include/general/*.h"
    "include/window_handler/*.h"
    "include/physics/*.h"
    "include/ships/*.h")

# Añadir todos los archivos fuente (recursivamente)
file(GLOB_RECURSE SOURCES "source/*.cpp")

# Añadir el ejecutable
add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCES})

# Enlazar las bibliotecas de SFML
target_link_libraries(${PROJECT_NAME} PRIVATE ImGui-SFML::ImGui-SFML sfml-graphics sfml-window sfml-system nlohmann_json::nlohmann_json)


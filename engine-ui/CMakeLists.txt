cmake_minimum_required(VERSION 3.10)
project(engine-ui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Root directories
set(ENGINE_ROOT ${CMAKE_SOURCE_DIR})
set(THIRDPARTY_DIR ${ENGINE_ROOT}/thirdparty)

# SDL2 include and lib paths
set(SDL2_INCLUDE_DIR ${THIRDPARTY_DIR}/SDL2/include)
set(SDL2_LIB_DIR ${THIRDPARTY_DIR}/SDL2/lib)

include_directories(
    ${THIRDPARTY_DIR}/SDL2/include/SDL2
    ${THIRDPARTY_DIR}/SDL2/include
    ${THIRDPARTY_DIR}/imgui
    ${THIRDPARTY_DIR}/imgui/backends
    ${THIRDPARTY_DIR}
    ${ENGINE_ROOT}/src
)

# IMGUI sources
set(IMGUI_SOURCES
    ${THIRDPARTY_DIR}/imgui/imgui.cpp
    ${THIRDPARTY_DIR}/imgui/imgui_draw.cpp
    ${THIRDPARTY_DIR}/imgui/imgui_tables.cpp
    ${THIRDPARTY_DIR}/imgui/imgui_widgets.cpp
    ${THIRDPARTY_DIR}/imgui/backends/imgui_impl_sdl2.cpp
    ${THIRDPARTY_DIR}/imgui/backends/imgui_impl_sdlrenderer2.cpp
)

# Engine sources
file(GLOB ENGINE_SOURCES
    ${ENGINE_ROOT}/src/*.cpp
)

add_executable(${PROJECT_NAME}
    ${ENGINE_SOURCES}
    ${IMGUI_SOURCES}
)

# SDL2 libraries
target_link_directories(${PROJECT_NAME} PRIVATE ${SDL2_LIB_DIR})
target_link_libraries(${PROJECT_NAME} SDL2 SDL2main)




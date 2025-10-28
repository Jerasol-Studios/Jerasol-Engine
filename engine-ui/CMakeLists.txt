cmake_minimum_required(VERSION 3.10)
project(engine-ui)

set(CMAKE_CXX_STANDARD 17)

include_directories(
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends
    ${CMAKE_SOURCE_DIR}/thirdparty/SDL2/include
)

add_executable(engine-ui
    src/main.cpp
    src/ui.cpp
    src/compiler_interface.cpp
    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/backends/imgui_impl_sdl2.cpp
    thirdparty/imgui/backends/imgui_impl_sdlrenderer2.cpp
)

target_link_directories(engine-ui PRIVATE ${CMAKE_SOURCE_DIR}/thirdparty/SDL2/lib)
target_link_libraries(engine-ui SDL2main SDL2)

# SDL2
set(SDL2_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/SDL2/include")
set(SDL2_LIB_DIR "${CMAKE_SOURCE_DIR}/thirdparty/SDL2/lib")

include_directories(${SDL2_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR})

# Link SDL2
target_link_libraries(engine-ui PRIVATE
    SDL2main
    SDL2
)

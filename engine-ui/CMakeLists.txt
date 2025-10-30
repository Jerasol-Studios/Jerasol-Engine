cmake_minimum_required(VERSION 3.20)
project(engine-ui LANGUAGES C CXX)

# ======================================
#  Basic Configuration
# ======================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/output)

# ======================================
#  Detect Build Mode
# ======================================
if(NOT DEFINED ENGINE_BUILD_MODE)
    set(ENGINE_BUILD_MODE "GUI")
endif()

message(STATUS "-- Using build mode: ${ENGINE_BUILD_MODE}")

if(${ENGINE_BUILD_MODE} STREQUAL "GUI")
    add_definitions(-DAPP_MODE_GUI)
else()
    add_definitions(-DAPP_MODE_CONSOLE)
endif()

# ======================================
#  Include Directories
# ======================================
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui
    ${CMAKE_SOURCE_DIR}/thirdparty/imgui/backends
    ${CMAKE_SOURCE_DIR}/thirdparty/SDL2/include
    ${CMAKE_SOURCE_DIR}/thirdparty/json
)

# ======================================
#  Source Files
# ======================================
set(SOURCES
    src/main.cpp
    src/ui.cpp
    src/flowchart.cpp
    src/compiler_interface.cpp

    thirdparty/imgui/imgui.cpp
    thirdparty/imgui/imgui_draw.cpp
    thirdparty/imgui/imgui_tables.cpp
    thirdparty/imgui/imgui_widgets.cpp
    thirdparty/imgui/imgui_demo.cpp

    thirdparty/imgui/backends/imgui_impl_sdl2.cpp
    thirdparty/imgui/backends/imgui_impl_sdlrenderer2.cpp
)

# ======================================
#  Executable Target
# ======================================
add_executable(engine-ui ${SOURCES})

# ======================================
#  SDL2 Linking
# ======================================
set(SDL2_PATH "${CMAKE_SOURCE_DIR}/thirdparty/SDL2")

if(WIN32)
    target_link_libraries(engine-ui
        ${SDL2_PATH}/lib/libSDL2.a
        ${SDL2_PATH}/lib/libSDL2main.a
        imm32
        setupapi
        version
        winmm
    )
else()
    find_package(SDL2 REQUIRED)
    target_link_libraries(engine-ui ${SDL2_LIBRARIES})
endif()

# ======================================
#  Output Info
# ======================================
message(STATUS "-- =====================================")
message(STATUS "-- Engine-UI Build Configured Successfully")
message(STATUS "-- Output Directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "-- =====================================")

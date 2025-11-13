cmake_minimum_required(VERSION 3.20)
project(engine_ui LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(APP_MODE "GUI or CONSOLE" "GUI") # pass -DAPP_MODE=CONSOLE to build console mode

# paths
set(THIRD_PARTY ${CMAKE_SOURCE_DIR}/thirdparty)
set(SDL2_DIR ${THIRD_PARTY}/SDL2)
set(IMGUI_DIR ${THIRD_PARTY}/imgui)
set(JSON_HPP ${THIRD_PARTY}/json.hpp)

# includes
include_directories(${IMGUI_DIR})
include_directories(${THIRD_PARTY})
include_directories(${IMGUI_DIR}/backends)
include_directories(${SDL2_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${SDL2_DIR}/include/SDL2)

# source files
file(GLOB IMGUI_SRC
    ${IMGUI_DIR}/*.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
)

file(GLOB SRC
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

add_executable(engine-ui ${SRC} ${IMGUI_SRC})

# definitions for subsystem (avoid console window when GUI)
if (APP_MODE STREQUAL "GUI")
    if (WIN32)
        set_target_properties(engine-ui PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS")
    endif()
endif()

# link SDL2 static lib if present
# Prefer prebuilt libs at thirdparty/SDL2/lib
if (EXISTS "${SDL2_DIR}/lib/libSDL2.a")
    target_link_libraries(engine-ui PRIVATE "${SDL2_DIR}/lib/libSDL2.a")
endif()

# link Win32 libs required by SDL2
if (WIN32)
    target_link_libraries(engine-ui PRIVATE Ws2_32 winmm imm32 setupapi Version)
endif()

# copy runtime DLL (SDL2.dll) next to exe if present
add_custom_command(TARGET engine-ui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/output
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:engine-ui> ${CMAKE_BINARY_DIR}/output/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${SDL2_DIR}/lib/SDL2.dll" ${CMAKE_BINARY_DIR}/output/ || true
)

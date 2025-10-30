cmake_minimum_required(VERSION 3.20)
project(engine_ui LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Root layout (adjust if your root differs)
set(PROJ_ROOT ${CMAKE_SOURCE_DIR})
set(SDL2_ROOT ${PROJ_ROOT}/thirdparty/SDL2)
set(IMGUI_ROOT ${PROJ_ROOT}/thirdparty/imgui)

include_directories(
    ${IMGUI_ROOT}/
    ${IMGUI_ROOT}/backends
    ${SDL2_ROOT}/include/SDL2
    ${PROJ_ROOT}/src
)

# ImGui sources (core)
set(IMGUI_SOURCES
    ${IMGUI_ROOT}/imgui.cpp
    ${IMGUI_ROOT}/imgui_demo.cpp
    ${IMGUI_ROOT}/imgui_draw.cpp
    ${IMGUI_ROOT}/imgui_widgets.cpp
    ${IMGUI_ROOT}/imgui_tables.cpp
)

# ImGui backend sources (we use SDL renderer 2 backend)
set(IMGUI_BACKENDS
    ${IMGUI_ROOT}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_ROOT}/backends/imgui_impl_sdlrenderer2.cpp
)

# Your app sources
set(SRC
    src/main.cpp
    src/ui.cpp
    src/Compiler_Interface.cpp
)

add_executable(engine-ui WIN32 ${SRC} ${IMGUI_SOURCES} ${IMGUI_BACKENDS})

# Link flags / libraries (link SDL2 from thirdparty)
target_link_directories(engine-ui PRIVATE ${SDL2_ROOT}/lib)
target_link_libraries(engine-ui PRIVATE SDL2 SDL2main)

# Copy SDL2.dll to output dir on build
add_custom_command(TARGET engine-ui POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory $<TARGET_FILE_DIR:engine-ui>/output
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SDL2_ROOT}/lib/SDL2.dll
        $<TARGET_FILE_DIR:engine-ui>/SDL2.dll
)

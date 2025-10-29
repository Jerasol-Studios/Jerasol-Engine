cmake_minimum_required(VERSION 3.20)
project(engine-ui)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Detect build mode
if(DEFINED APP_MODE)
    message(STATUS "Using build mode: ${APP_MODE}")
else()
    set(APP_MODE "GUI")
endif()

message(STATUS "-- Using build mode: ${APP_MODE}")

# ==========================================
# Directory Setup
# ==========================================
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(THIRDPARTY_DIR "${CMAKE_SOURCE_DIR}/thirdparty")
set(OUTPUT_DIR "${CMAKE_SOURCE_DIR}/build/output")

file(MAKE_DIRECTORY ${OUTPUT_DIR})

# ==========================================
# Include Paths
# ==========================================
include_directories(
    ${SRC_DIR}
    ${THIRDPARTY_DIR}
    ${THIRDPARTY_DIR}/SDL2/include/
    ${THIRDPARTY_DIR}/SDL2/include/SDL2/
    ${THIRDPARTY_DIR}/imgui
)

# ==========================================
# SDL2 Config
# ==========================================
set(SDL2_LIB_DIR "${THIRDPARTY_DIR}/SDL2/lib")
link_directories(${SDL2_LIB_DIR})

# ==========================================
# ImGui Sources
# ==========================================
set(IMGUI_DIR "${THIRDPARTY_DIR}/imgui")
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# ==========================================
# Main Sources
# ==========================================
set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/ui.cpp
    ${SRC_DIR}/Compiler_Interface.cpp
    ${IMGUI_SOURCES}
)

# ==========================================
# Build Executable
# ==========================================
add_executable(engine-ui ${SOURCES})

# ==========================================
# Link Libraries
# ==========================================
if(WIN32)
    target_link_libraries(engine-ui PRIVATE
        SDL2main
        SDL2
        opengl32
    )
else()
    target_link_libraries(engine-ui PRIVATE
        SDL2
        GL
        pthread
        dl
    )
endif()

# ==========================================
# Output Location
# ==========================================
set_target_properties(engine-ui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR}
)

# ==========================================
# Copy SDL2 DLL and Dependencies
# ==========================================
if(WIN32)
    add_custom_command(TARGET engine-ui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_LIB_DIR}/SDL2.dll"
        "${OUTPUT_DIR}/SDL2.dll"
    )
endif()

# ==========================================
# Final Info
# ==========================================
message(STATUS "=====================================")
message(STATUS "Engine-UI Build Configured Successfully")
message(STATUS "Output Directory: ${OUTPUT_DIR}")
message(STATUS "=====================================")

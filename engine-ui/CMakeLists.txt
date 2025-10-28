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

# ==========================================
# SDL2 Configuration
# ==========================================
set(SDL2_DIR "${CMAKE_SOURCE_DIR}/thirdparty/SDL2")
set(SDL2_INCLUDE_DIR "${SDL2_DIR}/include")
set(SDL2_LIB_DIR "${SDL2_DIR}/lib")

include_directories(${SDL2_INCLUDE_DIR})
link_directories(${SDL2_LIB_DIR})

# ==========================================
# ImGui Configuration
# ==========================================
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/thirdparty/imgui")
include_directories(${IMGUI_DIR})

set(IMGUI_SOURCES
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp
)

# ==========================================
# Source Files
# ==========================================
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

set(SOURCES
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/ui.cpp
    ${SRC_DIR}/Compiler_Interface.cpp
    ${IMGUI_SOURCES}
)

include_directories(${SRC_DIR})

# ==========================================
# Executable
# ==========================================
add_executable(engine-ui ${SOURCES})

# ==========================================
# Libraries
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
# Copy SDL2 DLL (Windows Only)
# ==========================================
if(WIN32)
    add_custom_command(TARGET engine-ui POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_LIB_DIR}/SDL2.dll"
        $<TARGET_FILE_DIR:engine-ui>
    )
endif()

# ==========================================
# Output Folder
# ==========================================
set_target_properties(engine-ui PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build/bin"
)

message(STATUS "Configured engine-ui successfully with SDL2 and ImGui.")


#pragma once
#include "imgui.h"
#include <string>

void RenderUI();
void SetTheme(bool dark);

// Compiler UI helpers (implemented in compiler_interface.cpp)
void ShowCompilerOutputUI();

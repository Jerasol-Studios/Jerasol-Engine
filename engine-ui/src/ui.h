#pragma once
#include "imgui.h"
#include <string>

void RenderUI();
void SetTheme(bool dark);
void RunCompiler(const std::string& sourcePath);
void ShowCompilerOutput();

#pragma once
#include "imgui.h"
#include <string>

void RenderUI();                     // draws the main UI window
void SetTheme(bool dark);            // apply dark/light theme
void RunCompiler(const std::string& sourcePath); // wrapper call to Compiler_Interface
void ShowCompilerOutput();           // show compiler output window
void ClearCompilerOutput();


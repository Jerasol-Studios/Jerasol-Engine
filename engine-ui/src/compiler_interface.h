#pragma once
#include <string>

struct Settings {
    std::string mingw_path; // full path to g++.exe
};

Settings& GetSettings();
void LoadSettings();
void SaveSettings();

// Run compiler and capture output shown in UI
void RunCompilerAndCapture(const std::string& sourceFile);

// Output UI
void ShowCompilerOutputUI();
void ClearCompilerOutput();

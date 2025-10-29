#include "Compiler_Interface.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include <filesystem>
#include <cstdio>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

static std::string compilerOutput;
static std::atomic<bool> compilerBusy{false};

// internal worker
static void CompilerThread(std::string sourcePath, std::string outputExePath)
{
    compilerBusy = true;
    compilerOutput.clear();

    fs::path src = sourcePath;
    fs::path out = outputExePath;

    if (!fs::exists(src)) {
        compilerOutput = "[Compiler] Error: Source file not found.\n";
        compilerBusy = false;
        return;
    }

    fs::path mingwPath = fs::current_path() / "mingw64" / "bin" / "g++.exe";
    if (!fs::exists(mingwPath)) {
        compilerOutput = "[Compiler] Error: g++.exe not found in mingw64/bin.\n";
        compilerBusy = false;
        return;
    }

    // Ensure output directory exists
    fs::create_directories(out.parent_path());

    std::stringstream cmd;
    cmd << "\"" << mingwPath.string() << "\" "
        << "\"" << src.string() << "\" "
        << "-o \"" << out.string() << "\" "
        << "-std=c++17 -static-libgcc -static-libstdc++";

    compilerOutput += "[Compiler] Running g++...\n";

#ifdef _WIN32
    FILE* pipe = _popen(cmd.str().c_str(), "r");
#else
    FILE* pipe = popen(cmd.str().c_str(), "r");
#endif

    if (!pipe) {
        compilerOutput += "[Compiler] Failed to start g++.\n";
        compilerBusy = false;
        return;
    }

    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe)) {
        compilerOutput += buffer;
    }

#ifdef _WIN32
    int result = _pclose(pipe);
#else
    int result = pclose(pipe);
#endif

    if (result == 0)
        compilerOutput += "\n[Compiler] Compilation successful.\n";
    else
        compilerOutput += "\n[Compiler] Compilation failed.\n";

    compilerBusy = false;
}

void RunCompilerAsync(const std::string& sourcePath, const std::string& outputExePath)
{
    if (compilerBusy) {
        compilerOutput = "[Compiler] Already compiling.\n";
        return;
    }
    std::thread(CompilerThread, sourcePath, outputExePath).detach();
}

bool IsCompilerBusy() { return compilerBusy; }
const std::string& GetCompilerOutput() { return compilerOutput; }
void ClearCompilerOutput() { compilerOutput.clear(); }

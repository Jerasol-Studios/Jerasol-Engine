#include "compiler_interface.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <sstream>        // ✅ FIX: added this line
#include <cstdlib>
#include <cstdio>
#include <thread>
#include <chrono>

namespace fs = std::filesystem;

CompilerInterface::CompilerInterface() {
    // Attempt to auto-detect MinGW paths
    mingwPath = "C:/Users/student/Documents/Jerasol-Engine-Program-Code/engine-ui/mingw64/bin/";
    gppPath = mingwPath + "g++.exe";
    makePath = mingwPath + "mingw32-make.exe";
}

bool CompilerInterface::CompileCode(const std::string& inputFile, const std::string& outputFile) {
    std::string command = "\"" + gppPath + "\" \"" + inputFile + "\" -o \"" + outputFile + "\" 2>&1";

    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        lastOutput = "[Compiler] Failed to start compilation process.";
        return false;
    }

    char buffer[256];
    std::string captured;
    while (fgets(buffer, sizeof(buffer), pipe)) {
        captured += buffer;
    }

    _pclose(pipe);
    lastOutput = captured;

    // ✅ Display output nicely in the console/log system
    std::ostringstream formatted;
    std::istringstream iss(captured);
    std::string line;
    while (std::getline(iss, line)) {
        formatted << "[Compiler] " << line << "\n";
    }

    lastOutput = formatted.str();
    return (captured.find("error") == std::string::npos);
}

bool CompilerInterface::RunExecutable(const std::string& exePath, const std::string& workingDir) {
    if (!fs::exists(exePath)) {
        lastOutput = "[Runtime] Executable not found: " + exePath;
        return false;
    }

    std::string command = "cd /d \"" + workingDir + "\" && \"" + exePath + "\" 2>&1";

    FILE* pipe = _popen(command.c_str(), "r");
    if (!pipe) {
        lastOutput = "[Runtime] Failed to run executable.";
        return false;
    }

    char buffer[256];
    std::string captured;
    while (fgets(buffer, sizeof(buffer), pipe)) {
        captured += buffer;
    }

    _pclose(pipe);

    std::ostringstream formatted;
    std::istringstream iss(captured);
    std::string line;
    while (std::getline(iss, line)) {
        formatted << "[Runtime] " << line << "\n";
    }

    lastOutput = formatted.str();
    return true;
}

std::string CompilerInterface::GetLastOutput() const {
    return lastOutput;
}

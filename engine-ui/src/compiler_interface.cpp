#include "compiler_interface.h"
#include <filesystem>
#include <cstdio>
#include <windows.h>
#include <iostream>

std::string compilerOutput;

void RunCompiler(const std::string& sourceCode)
{
    namespace fs = std::filesystem;
    compilerOutput.clear();
    compilerOutput += "[Compiler] Running g++...\n";

    try {
        // Determine executable directory
        char exePath[MAX_PATH];
        GetModuleFileNameA(NULL, exePath, MAX_PATH);
        fs::path exeDir = fs::path(exePath).parent_path();

        // Path to compiler
        fs::path gppPath = exeDir / "mingw64" / "bin" / "g++.exe";
        if (!fs::exists(gppPath)) {
            compilerOutput += "[ERROR] g++.exe not found! Expected at:\n";
            compilerOutput += gppPath.string() + "\n";
            return;
        }

        // Prepare temp build directory
        fs::path tempDir = exeDir / "build" / "temp";
        fs::create_directories(tempDir);

        fs::path tempSource = tempDir / "user_code.cpp";
        fs::path outExe = tempDir / "user_program.exe";

        // Write source code to file
        FILE* f = fopen(tempSource.string().c_str(), "w");
        if (!f) {
            compilerOutput += "[ERROR] Could not create temp source file.\n";
            return;
        }
        fwrite(sourceCode.c_str(), 1, sourceCode.size(), f);
        fclose(f);

        // Build compile command
        std::string command = "\"" + gppPath.string() + "\" \"" + tempSource.string() +
                              "\" -o \"" + outExe.string() + "\" 2>&1";

        compilerOutput += "Command: " + command + "\n";

        // Run compiler and capture output
        FILE* pipe = _popen(command.c_str(), "r");
        if (!pipe) {
            compilerOutput += "[ERROR] Failed to start compiler process.\n";
            return;
        }

        char buffer[256];
        while (fgets(buffer, sizeof(buffer), pipe))
            compilerOutput += buffer;
        int result = _pclose(pipe);

        if (result == 0) {
            compilerOutput += "\n[Compiler] Compilation successful.\n";
            compilerOutput += "[Running program...]\n";

            // Execute compiled program
            FILE* progPipe = _popen(outExe.string().c_str(), "r");
            if (progPipe) {
                while (fgets(buffer, sizeof(buffer), progPipe))
                    compilerOutput += buffer;
                _pclose(progPipe);
            }
            compilerOutput += "\n[Program finished.]\n";
        } else {
            compilerOutput += "\n[Compiler] Compilation failed.\n";
        }

    } catch (const std::exception& e) {
        compilerOutput += std::string("[EXCEPTION] ") + e.what() + "\n";
    }
}


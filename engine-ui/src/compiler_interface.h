#pragma once
#include <string>

// Compile a source file into an executable.
// - sourcePath: path to the .cpp file (input)
// - exePath: desired output executable path
// - mingwGppPath: path to g++.exe (relative or absolute). Example: "../mingw64/bin/g++.exe"
// - outLog: receives the full build log (stdout+stderr contents).
// Returns: the process exit code (0 = success typically).
int compileWithMinGW(const std::string& sourcePath,
                     const std::string& exePath,
                     const std::string& mingwGppPath,
                     const std::string& extraFlags,
                     std::string& outLog);

// Run an executable without opening a console window.
// - exePath: path to the exe to run
// - workingDir: working directory for the process (can be "")
// - outError: receives an error message if spawning fails
// Returns: true if the process was launched successfully.
bool runExecutableNoConsole(const std::string& exePath,
                            const std::string& workingDir,
                            std::string& outError);


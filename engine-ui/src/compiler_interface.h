#pragma once
#include <string>

// Run compiler for `sourcePath`. Returns combined stdout+stderr text from the compiler.
std::string RunCompilerCommand(const std::string& sourcePath);

// Run the compiled program (path to exe). Returns true if process launched.
bool RunCompiledProgram(const std::string& exePath);

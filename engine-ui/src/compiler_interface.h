#pragma once
#include <string>
#include <atomic>

// Starts compiling the given source file asynchronously and outputs the exe
// to the specified path. Returns immediately; use IsCompilerBusy() to check.
void RunCompilerAsync(const std::string& sourcePath, const std::string& outputExePath);

// Returns true if a compilation is currently running.
bool IsCompilerBusy();

// Returns the full compiler console log (output + errors).
const std::string& GetCompilerOutput();

// Clears the stored compiler log.
void ClearCompilerOutput();

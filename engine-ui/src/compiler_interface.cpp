#include "Compiler_Interface.h"
#include <windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <filesystem>

static const char* COMPILER_PATH = "mingw64\\bin\\g++.exe"; // relative to project root
static const char* OUTPUT_DIR = "build\\output";
static const char* OUTPUT_EXE = "build\\output\\program.exe";

static std::string ReadPipeToString(HANDLE pipe)
{
    std::string result;
    constexpr DWORD bufSize = 4096;
    char buffer[bufSize];
    DWORD read = 0;
    while (true) {
        BOOL ok = ReadFile(pipe, buffer, bufSize - 1, &read, nullptr);
        if (!ok || read == 0) break;
        result.append(buffer, buffer + read);
    }
    return result;
}

std::string RunCompilerCommand(const std::string& sourcePath)
{
    // Ensure output dir exists
    std::error_code ec;
    std::filesystem::create_directories(OUTPUT_DIR, ec);

    // Command: "mingw64\bin\g++.exe" -std=c++17 -O2 -o "build\output\program.exe" "path\to\source.cpp"
    std::ostringstream cmd;
    cmd << "\"" << COMPILER_PATH << "\""
        << " -std=c++17 -O2 -static -g -pipe"
        << " -o \"" << OUTPUT_EXE << "\""
        << " \"" << sourcePath << "\"";

    std::string commandLine = cmd.str();

    // Setup pipes for stdout and stderr
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(sa);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE stdoutRead = nullptr, stdoutWrite = nullptr;
    HANDLE stderrRead = nullptr, stderrWrite = nullptr;

    if (!CreatePipe(&stdoutRead, &stdoutWrite, &sa, 0)) {
        return "[Compiler] Failed to create stdout pipe.\n";
    }
    if (!SetHandleInformation(stdoutRead, HANDLE_FLAG_INHERIT, 0)) {
        // continue
    }
    if (!CreatePipe(&stderrRead, &stderrWrite, &sa, 0)) {
        CloseHandle(stdoutRead); CloseHandle(stdoutWrite);
        return "[Compiler] Failed to create stderr pipe.\n";
    }
    if (!SetHandleInformation(stderrRead, HANDLE_FLAG_INHERIT, 0)) {
        // continue
    }

    // Prepare STARTUPINFO to redirect handles
    STARTUPINFOA si{};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = stdoutWrite;
    si.hStdError  = stderrWrite;
    si.wShowWindow = SW_HIDE; // hide child console

    PROCESS_INFORMATION pi{};

    // Create process
    // CreateProcessA expects command line editable buffer
    std::vector<char> cmdLineVec(commandLine.begin(), commandLine.end());
    cmdLineVec.push_back('\0');

    BOOL ok = CreateProcessA(
        nullptr,
        cmdLineVec.data(),
        nullptr, nullptr,
        TRUE,
        CREATE_NO_WINDOW,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    // close write ends in parent so we can read EOF when child exits
    CloseHandle(stdoutWrite);
    CloseHandle(stderrWrite);

    std::string output;
    if (!ok) {
        DWORD err = GetLastError();
        std::ostringstream e;
        e << "[Compiler] Failed to spawn compiler (CreateProcess). Error: " << err << "\nCommand: " << commandLine << "\n";
        output = e.str();
        CloseHandle(stdoutRead); CloseHandle(stderrRead);
        return output;
    }

    // Read child output while it's running
    // We'll poll process; read available pipe content repeatedly
    for (;;) {
        DWORD status = WaitForSingleObject(pi.hProcess, 50); // 50ms wait
        // Read whatever is available
        output += ReadPipeToString(stdoutRead);
        output += ReadPipeToString(stderrRead);
        if (status == WAIT_OBJECT_0) break;
    }

    // Ensure we read final data
    output += ReadPipeToString(stdoutRead);
    output += ReadPipeToString(stderrRead);

    // Close handles
    CloseHandle(stdoutRead);
    CloseHandle(stderrRead);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    // If output exe exists, append success message
    if (std::filesystem::exists(OUTPUT_EXE)) {
        output += "\n[Compiler] Success — executable generated at: ";
        output += OUTPUT_EXE;
        output += "\n";
    } else {
        output += "\n[Compiler] Compilation finished — executable not created.\n";
    }

    return output;
}

bool RunCompiledProgram(const std::string& exePath)
{
    if (!std::filesystem::exists(exePath)) return false;

    STARTUPINFOA si{};
    PROCESS_INFORMATION pi{};
    si.cb = sizeof(si);
    si.dwFlags = 0;
    std::string cmd = "\"" + exePath + "\"";

    std::vector<char> cmdvec(cmd.begin(), cmd.end());
    cmdvec.push_back('\0');

    BOOL ok = CreateProcessA(nullptr, cmdvec.data(), nullptr, nullptr, FALSE, 0, nullptr, nullptr, &si, &pi);
    if (!ok) {
        return false;
    }
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
}

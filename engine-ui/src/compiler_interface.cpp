#include "compiler_interface.h"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace fs = std::filesystem;

// Helper: read whole file
static std::string readFileToString(const std::string& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return std::string();
    std::ostringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

// Helper: ensure parent directory exists
static void ensureParentDir(const std::string& path) {
    fs::path p = fs::path(path).parent_path();
    if (!p.empty() && !fs::exists(p)) fs::create_directories(p);
}

// Build command and run it redirecting stdout+stderr to a log file, then read the log.
int compileWithMinGW(const std::string& sourcePath,
                     const std::string& exePath,
                     const std::string& mingwGppPath,
                     const std::string& extraFlags,
                     std::string& outLog)
{
    outLog.clear();

    // Paths
    std::string logPath = "logs/build_tool.log";
    ensureParentDir(logPath);
    ensureParentDir(exePath);

    // Quote paths for Windows/space-safety
    auto quote = [](const std::string& s){
        std::string r = "\"";
        for (char c : s) {
            if (c == '\\') r += '/'; // use forward slashes for system command safety
            r.push_back(c);
        }
        r += "\"";
        return r;
    };

    // Build command:
    // "<g++>" -std=c++17 -O2 -static-libgcc -static-libstdc++ -o "<exe>" "<source>" <extra> > logs/build_tool.log 2>&1
    std::ostringstream cmd;
    cmd << quote(mingwGppPath)
        << " -std=c++17 "
        << extraFlags << " "
        << "-o " << quote(exePath) << " "
        << quote(sourcePath)
        << " > " << quote(logPath) << " 2>&1";

    // Run command
    int rc = std::system(cmd.str().c_str());

    // read log
    outLog = readFileToString(logPath);

    // on Windows, system() will return the exit code in a platform-dependent way; we return rc directly.
    return rc;
}

bool runExecutableNoConsole(const std::string& exePath,
                            const std::string& workingDir,
                            std::string& outError)
{
    outError.clear();

#ifdef _WIN32
    // Use CreateProcessA and CREATE_NO_WINDOW so no console pops up.
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Prepare command line: surround path in quotes
    std::string cmdLine = "\"" + exePath + "\"";

    // working dir can be empty (use current)
    LPCSTR lpWorkDir = nullptr;
    std::string workDirLocal;
    if (!workingDir.empty()) { workDirLocal = workingDir; lpWorkDir = workDirLocal.c_str(); }

    BOOL ok = CreateProcessA(
        nullptr,
        cmdLine.data(), // NOTE: CreateProcess may modify this buffer in place
        nullptr,
        nullptr,
        FALSE,
        CREATE_NO_WINDOW,
        nullptr,
        lpWorkDir,
        &si,
        &pi
    );

    if (!ok) {
        DWORD err = GetLastError();
        std::ostringstream ss;
        ss << "CreateProcess failed (code " << err << ")";
        outError = ss.str();
        return false;
    }

    // We don't wait: let it run detached. Close handles.
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return true;
#else
    // Fallback for non-windows — launch in background
    std::string cmd = "\"" + exePath + "\" &";
    int rc = std::system(cmd.c_str());
    if (rc != 0) {
        outError = "Failed to launch executable (non-windows path).";
        return false;
    }
    return true;
#endif
}

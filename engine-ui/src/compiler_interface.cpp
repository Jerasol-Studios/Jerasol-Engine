// compiler_interface.cpp
#include "compiler_interface.h"
#include "imgui.h"
#include "json.hpp"

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>
#include <mutex>
#include <fstream>
#include <sstream>
#include <filesystem>

using json = nlohmann::json;
static Settings g_settings;
static std::vector<std::string> g_output_lines;
static std::mutex g_output_mutex;

Settings& GetSettings() { return g_settings; }

void LoadSettings() {
    std::ifstream in("settings.json");
    if (!in.is_open()) {
        g_settings.mingw_path = std::filesystem::current_path().string() + "/mingw64/bin/g++.exe";
        return;
    }
    json j; in >> j;
    if (j.contains("mingw_path")) g_settings.mingw_path = j["mingw_path"].get<std::string>();
}

void SaveSettings() {
    json j;
    j["mingw_path"] = g_settings.mingw_path;
    std::ofstream out("settings.json");
    out << j.dump(2);
}

void AppendOutputLine(const std::string& line) {
    std::lock_guard<std::mutex> lock(g_output_mutex);
    g_output_lines.push_back(line);
}

void ClearCompilerOutput() {
    std::lock_guard<std::mutex> lock(g_output_mutex);
    g_output_lines.clear();
}

static std::string ExecCommandCapture(const std::string& cmd) {
    std::string result;
#ifdef _WIN32
    FILE* pipe = _popen(cmd.c_str(), "r");
#else
    FILE* pipe = popen(cmd.c_str(), "r");
#endif
    if (!pipe) return "Failed to run command.";
    char buffer[512];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
    return result;
}

void RunCompilerAndCapture(const std::string& sourceFile) {
    ClearCompilerOutput();
    std::string gpp = g_settings.mingw_path;
    if (gpp.empty()) {
        AppendOutputLine("[Compiler] g++ path not set. Open Settings and set mingw path.");
        return;
    }
    if (!std::filesystem::exists(gpp)) {
        AppendOutputLine("[Compiler] g++.exe not found at: " + gpp);
        return;
    }
    std::string outExe = std::filesystem::current_path().string() + "/build/output/compiled_temp.exe";
    std::string cmd = "\"" + gpp + "\" -g -std=c++17 -O2 -o \"" + outExe + "\" \"" + sourceFile + "\" 2>&1";
    AppendOutputLine("[Compiler] Running: " + cmd);
    std::string output = ExecCommandCapture(cmd);
    if (output.empty()) output = "[Compiler] No output.";
    std::istringstream ss(output);
    std::string line;
    while (std::getline(ss, line)) {
        AppendOutputLine(line);
    }
}

void ShowCompilerOutputUI() {
    ImGui::BeginChild("compiler_output_child", ImVec2(-1, 200), true);
    ImGui::Columns(2, nullptr, false);
    ImGui::SetColumnWidth(0, ImGui::GetWindowWidth() - 140);
    {
        std::lock_guard<std::mutex> lock(g_output_mutex);
        for (auto &l : g_output_lines) {
            ImGui::TextWrapped("%s", l.c_str());
        }
    }
    ImGui::NextColumn();
    if (ImGui::Button("Copy")) {
        // build a single string
        std::string all;
        {
            std::lock_guard<std::mutex> lock(g_output_mutex);
            for (auto &l : g_output_lines) { all += l + "\r\n"; }
        }
        ImGui::SetClipboardText(all.c_str());
    }
    if (ImGui::Button("Clear")) ClearCompilerOutput();
    ImGui::EndChild();
}


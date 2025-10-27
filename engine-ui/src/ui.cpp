#include "ui.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdio>
#include <array>
#include <memory>
#include <sstream>
#include <filesystem>
#include <algorithm> // <-- Needed for std::clamp

static bool showFileMenu = false;
static bool showSettings = false;
static bool darkMode = true;
static float panelWidth = 180.0f;
static int activePage = 0;

// --- Compiler Data ---
static std::vector<std::string> compilerOutput;
static std::string codeBuffer =
R"(#include <iostream>
int main() {
    std::cout << "Hello from Engine Compiler!" << std::endl;
    return 0;
})";
static std::string lastExecutablePath;

// ----------------------------------------------------------
void SetTheme(bool dark)
{
    if (dark) ImGui::StyleColorsDark();
    else ImGui::StyleColorsLight();
}

// Helper: Execute command
std::string ExecCommand(const std::string& cmd)
{
    std::array<char, 256> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd.c_str(), "r"), _pclose);
    if (!pipe) return "[Error] Failed to open pipe.";
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
        result += buffer.data();
    return result;
}

// Compiler
void RunCompiler(const std::string& sourceCode)
{
    compilerOutput.clear();

    namespace fs = std::filesystem;
    fs::path buildDir = fs::current_path() / "build";
    if (!fs::exists(buildDir))
        fs::create_directory(buildDir);

    fs::path sourceFile = buildDir / "temp_code.cpp";
    fs::path outputFile = buildDir / "temp_code.exe";

    std::ofstream out(sourceFile);
    out << sourceCode;
    out.close();

    compilerOutput.push_back("[Compiler] Running g++...");
    std::string cmd = "\"../mingw64/bin/g++.exe\" \"" + sourceFile.string() + "\" -o \"" + outputFile.string() + "\" 2>&1";
    std::string result = ExecCommand(cmd);
    if (!result.empty()) compilerOutput.push_back(result);

    if (fs::exists(outputFile))
    {
        compilerOutput.push_back("[Compiler] Compilation successful!");
        lastExecutablePath = outputFile.string();
    }
    else
    {
        compilerOutput.push_back("[Compiler] Compilation failed.");
        lastExecutablePath.clear();
    }
}

// Runner
void RunProgram()
{
    compilerOutput.push_back("[Runner] Executing program...");
    if (lastExecutablePath.empty())
    {
        compilerOutput.push_back("[Error] No compiled program found!");
        return;
    }

    std::string cmd = "\"" + lastExecutablePath + "\"";
    std::string output = ExecCommand(cmd);
    compilerOutput.push_back(output);
}

// ----------------------------------------------------------
void RenderUI()
{
    ImVec2 displaySize = ImGui::GetIO().DisplaySize;
    const float topBarHeight = 30.0f;

    // Close File menu if clicked off
    if (showFileMenu &&
        !ImGui::IsAnyItemHovered() &&
        ImGui::IsMouseClicked(ImGuiMouseButton_Left) &&
        !ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow))
    {
        showFileMenu = false;
    }

    // Top Bar
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(displaySize.x, topBarHeight));
    ImGui::Begin("Top Bar", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

    if (ImGui::Button("FILE")) showFileMenu = !showFileMenu;
    ImGui::SameLine();
    if (ImGui::Button("SETTINGS")) showSettings = !showSettings;

    ImGui::End();

    // File Dropdown
    if (showFileMenu)
    {
        ImGui::SetNextWindowPos(ImVec2(8, topBarHeight));
        ImGui::Begin("File Menu", &showFileMenu,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoResize);
        if (ImGui::MenuItem("NEW", "Ctrl+N")) codeBuffer.clear();
        if (ImGui::MenuItem("SAVE", "Ctrl+S")) { std::ofstream("last_saved.cpp") << codeBuffer; }
        if (ImGui::MenuItem("SAVE AS", "Ctrl+Shift+S")) {}
        if (ImGui::MenuItem("OPEN", "Ctrl+O")) {}
        ImGui::End();
    }

    // Left Panel
    ImGui::SetNextWindowPos(ImVec2(0, topBarHeight));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, displaySize.y - topBarHeight));
    ImGui::Begin("Panel", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar);

    if (ImGui::Button("HOME", ImVec2(-1, 0))) activePage = 0;
    if (ImGui::Button("FLOWCHART", ImVec2(-1, 0))) activePage = 1;
    if (ImGui::Button("EDITOR", ImVec2(-1, 0))) activePage = 2;
    if (ImGui::Button("CODE EDITOR", ImVec2(-1, 0))) activePage = 3;
    ImGui::Separator();
    ImGui::Text("Resize panel ←→");
    ImGui::End();

    // Resizer
    ImGui::SetNextWindowPos(ImVec2(panelWidth - 3, topBarHeight));
    ImGui::SetNextWindowSize(ImVec2(6, displaySize.y - topBarHeight));
    ImGui::Begin("##resizer", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar);
    ImGui::InvisibleButton("##resizebtn", ImVec2(6, displaySize.y - topBarHeight));
    if (ImGui::IsItemActive())
        panelWidth = std::clamp(panelWidth + ImGui::GetIO().MouseDelta.x, 120.0f, 400.0f);
    ImGui::End();

    // Main Area
    ImGui::SetNextWindowPos(ImVec2(panelWidth, topBarHeight));
    ImGui::SetNextWindowSize(ImVec2(displaySize.x - panelWidth, displaySize.y - topBarHeight));
    ImGui::Begin("Main Area", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);

    switch (activePage)
    {
        case 0: ImGui::Text("🏠 Home Page"); break;
        case 1: ImGui::Text("📊 Flowchart Editor"); break;
        case 2: ImGui::Text("📝 Editor"); break;
        case 3:
        {
            ImGui::Text("💻 Code Editor");
            ImGui::Separator();

            static std::vector<char> textBuf(codeBuffer.begin(), codeBuffer.end());
            textBuf.resize(4096, '\0'); // Reserve some space

            if (ImGui::InputTextMultiline("##code", textBuf.data(), textBuf.size(),
                ImVec2(-1, 300), ImGuiInputTextFlags_AllowTabInput))
            {
                codeBuffer = textBuf.data();
            }

            if (ImGui::Button("Compile")) RunCompiler(codeBuffer);
            ImGui::SameLine();
            if (ImGui::Button("Run")) RunProgram();

            ImGui::Separator();
            ImGui::Text("Output:");
            ImGui::BeginChild("output", ImVec2(0, 200), true);
            for (auto& line : compilerOutput)
                ImGui::TextWrapped("%s", line.c_str());
            ImGui::EndChild();
        }
        break;
    }

    ImGui::End();

    // Settings
    if (showSettings)
    {
        ImGui::Begin("Settings", &showSettings);
        ImGui::Text("Preferences");
        if (ImGui::Checkbox("Dark Mode", &darkMode)) SetTheme(darkMode);
        ImGui::End();
    }
}

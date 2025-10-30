#include "ui.h"
#include "Compiler_Interface.h"
#include <windows.h>
#include <commdlg.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <imgui.h>

static std::string selectedSource;
static std::string compilerOutput;
static bool darkTheme = true;

// helper: open file dialog for .cpp
static std::string OpenFileDialogCPP()
{
    char filename[MAX_PATH] = "";
    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "C++ Source Files (*.cpp)\0*.cpp\0All Files (*.*)\0*.*\0";
    ofn.lpstrFile = filename;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    ofn.lpstrDefExt = "cpp";

    if (GetOpenFileNameA(&ofn)) {
        return std::string(filename);
    }
    return "";
}

void ClearCompilerOutput()
{
    compilerOutput.clear();
}

void RunCompiler(const std::string& sourcePath)
{
    if (sourcePath.empty()) {
        compilerOutput = "[Compiler] No source file selected.\n";
        return;
    }
    compilerOutput = RunCompilerCommand(sourcePath);
}

void ShowCompilerOutput()
{
    ImGui::Begin("Compiler Output", nullptr, ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Compiler Output");
    ImGui::Separator();

    if (ImGui::Button("Clear")) {
        ClearCompilerOutput();
    }
    ImGui::SameLine();
    if (ImGui::Button("Copy")) {
        ImGui::SetClipboardText(compilerOutput.c_str());
    }
    ImGui::SameLine();
    if (ImGui::Button("Recompile")) {
        RunCompiler(selectedSource);
    }

    ImGui::SameLine();
    if (ImGui::Button("Run Program")) {
        // use same output path as Compiler_Interface: build\output\program.exe
        RunCompiledProgram("build\\output\\program.exe");
    }

    ImGui::Dummy(ImVec2(0, 8));
    ImGui::BeginChild("compiler_scroll", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);
    if (compilerOutput.empty()) {
        ImGui::TextDisabled("No output yet.");
    } else {
        ImGui::TextUnformatted(compilerOutput.c_str());
    }
    ImGui::EndChild();
    ImGui::End();
}

void SetTheme(bool dark)
{
    darkTheme = dark;
    if (darkTheme) ImGui::StyleColorsDark();
    else ImGui::StyleColorsLight();
}

void RenderUI()
{
    // Main window - simple top bar and left resizable panel
    static float leftPanelWidth = 200.0f;
    static bool draggingPanel = false;

    ImGui::SetNextWindowPos(ImVec2(0,0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize, ImGuiCond_Always);

    ImGui::Begin("Engine-UI Main", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    // Top Bar
    ImGui::BeginChild("TopBar", ImVec2(0, 30), false);
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Open\tCtrl+O")) {
                std::string p = OpenFileDialogCPP();
                if (!p.empty()) selectedSource = p;
            }
            if (ImGui::MenuItem("Save\tCtrl+S")) {
                // hook later
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Settings")) {
            if (ImGui::MenuItem("Dark Mode", nullptr, darkTheme)) SetTheme(true);
            if (ImGui::MenuItem("Light Mode", nullptr, !darkTheme)) SetTheme(false);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    ImGui::EndChild();

    ImGui::Separator();

    // Content: left panel + separator + main area
    ImGui::BeginChild("LeftPanel", ImVec2(leftPanelWidth, 0), true);
    ImGui::Text("PANEL");
    ImGui::Separator();
    if (ImGui::Button("HOME")) {}
    if (ImGui::Button("FLOWCHART EDITOR")) {}
    if (ImGui::Button("CODE EDITOR")) {}
    ImGui::EndChild();

    // Panel resize handle
    ImGui::SameLine();
    ImGui::InvisibleButton("panel_resize", ImVec2(6, ImGui::GetContentRegionAvail().y));
    if (ImGui::IsItemActive()) {
        leftPanelWidth += ImGui::GetIO().MouseDelta.x;
        if (leftPanelWidth < 120.0f) leftPanelWidth = 120.0f;
        if (leftPanelWidth > 700.0f) leftPanelWidth = 700.0f;
    }
    ImGui::SameLine();

    // Main content area
    ImGui::BeginChild("MainArea", ImVec2(0, 0), false);
    ImGui::Text("Compiler Panel");
    ImGui::Separator();

    ImGui::Text("Selected Source:");
    if (selectedSource.empty()) {
        ImGui::TextColored(ImVec4(1,0.4f,0.4f,1.0f), "No file selected.");
    } else {
        ImGui::TextWrapped("%s", selectedSource.c_str());
    }

    ImGui::Dummy(ImVec2(0,8));
    if (ImGui::Button("Choose file")) {
        std::string p = OpenFileDialogCPP();
        if (!p.empty()) selectedSource = p;
    }
    ImGui::SameLine();
    if (ImGui::Button("Compile")) {
        RunCompiler(selectedSource);
        // capture last compiler output for display
        // we call ShowCompilerOutput window to display
    }

    ImGui::SameLine();
    if (ImGui::Button("Open Output Window")) {
        // will show in separate window below
    }

    // Simple toolbar area
    ImGui::Separator();
    ImGui::TextWrapped("Quick actions: Compile -> run -> inspect output.");

    ImGui::EndChild(); // MainArea

    ImGui::End(); // Engine-UI Main

    // Show output window (separate floating window)
    ShowCompilerOutput();
}


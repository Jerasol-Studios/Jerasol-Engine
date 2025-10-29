#include "ui.h"
#include "Compiler_Interface.h"
#include <fstream>
#include <filesystem>
#include <string>
#include <iostream>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

void SetTheme(bool dark)
{
    if (dark)
        ImGui::StyleColorsDark();
    else
        ImGui::StyleColorsLight();
}

void RenderUI()
{
    static float panelWidth = 160.0f;
    static bool settingsOpen = false;
    static bool showFileMenu = false;
    static bool darkMode = true;
    static char projectName[128] = "MyProject";

    // Left Panel
    ImGui::SetNextWindowPos(ImVec2(0, 20));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, ImGui::GetIO().DisplaySize.y - 20));
    ImGui::Begin("Panel", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Jerasol Engine");
    ImGui::Separator();
    ImGui::Button("HOME", ImVec2(-1, 30));
    ImGui::Button("FLOWCHART EDITOR", ImVec2(-1, 30));
    ImGui::Button("CODE EDITOR", ImVec2(-1, 30));

    ImGui::Dummy(ImVec2(0, 5));
    ImGui::Text("Resize Panel:");
    ImGui::SliderFloat("##panelWidth", &panelWidth, 120.0f, 400.0f);
    ImGui::End();

    // Top bar
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 20));
    ImGui::Begin("TopBar", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
                 ImGuiWindowFlags_NoScrollbar);

    if (ImGui::Button("FILE")) showFileMenu = !showFileMenu;
    ImGui::SameLine();
    if (ImGui::Button("SETTINGS")) settingsOpen = true;
    ImGui::End();

    if (showFileMenu) {
        ImGui::SetNextWindowPos(ImVec2(0, 20));
        ImGui::Begin("FileMenu", &showFileMenu,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                     ImGuiWindowFlags_NoMove);
        ImGui::Text("NEW (Ctrl+N)");
        ImGui::Text("OPEN (Ctrl+O)");
        ImGui::Text("SAVE (Ctrl+S)");
        ImGui::Text("SAVE AS (Ctrl+Shift+S)");
        ImGui::End();
    }

    if (settingsOpen) {
        ImGui::Begin("Settings", &settingsOpen);
        if (ImGui::TreeNode("Preferences")) {
            if (ImGui::Checkbox("Dark Mode", &darkMode))
                SetTheme(darkMode);
            ImGui::TreePop();
        }
        ImGui::End();
    }

    // Code editor
    static char codeText[8192] =
        "#include <iostream>\nint main(){ std::cout << \"Hello from Jerasol!\"; return 0; }";

    static std::string compileOutput;
    static bool showOutput = false;
    static std::string exePath;

    ImGui::SetNextWindowPos(ImVec2(panelWidth, 20));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - panelWidth,
                                    ImGui::GetIO().DisplaySize.y - 20));
    ImGui::Begin("Code Editor", nullptr,
                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    ImGui::InputText("Project Name", projectName, sizeof(projectName));
    ImGui::Separator();
    ImGui::InputTextMultiline("##code", codeText, sizeof(codeText),
                              ImVec2(-1, 300), ImGuiInputTextFlags_AllowTabInput);

    std::string srcPath = "build/output/projects/" + std::string(projectName) + "/source.cpp";
    exePath = "build/output/projects/" + std::string(projectName) + "/program.exe";

    if (!IsCompilerBusy()) {
        if (ImGui::Button("Compile")) {
            fs::create_directories(fs::path(srcPath).parent_path());
            std::ofstream ofs(srcPath);
            ofs << codeText;
            ofs.close();

            ClearCompilerOutput();
            RunCompilerAsync(srcPath, exePath);
            showOutput = true;
        }
    } else {
        ImGui::Text("Compiling...");
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear Output"))
        compileOutput.clear();

    compileOutput = GetCompilerOutput();

    if (showOutput) {
        ImGui::Separator();
        ImGui::Text("Compiler Output:");
        ImGui::BeginChild("compiler_output", ImVec2(-1, 150), true);
        ImGui::TextWrapped("%s", compileOutput.c_str());
        ImGui::EndChild();
    }

    // --- New Section: Run Executable Button ---
    if (fs::exists(exePath) && compileOutput.find("successful") != std::string::npos) {
        if (ImGui::Button("Run Executable")) {
#ifdef _WIN32
            std::string fullPath = fs::absolute(exePath).string();
            ShellExecuteA(nullptr, "open", fullPath.c_str(), nullptr, nullptr, SW_SHOW);
#else
            std::string fullPath = fs::absolute(exePath).string();
            std::system(fullPath.c_str());
#endif
        }
    }

    ImGui::End();
}


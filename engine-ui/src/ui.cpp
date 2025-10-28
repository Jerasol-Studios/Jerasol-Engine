#include "ui.h"
#include "compiler_interface.h"
#include "imgui.h"
#include <string>
#include <vector>
#include <algorithm>

static float panelWidth = 200.0f;
static bool darkMode = true;
static std::string codeBuffer =
R"(#include <iostream>
int main() {
    std::cout << "Hello from built-in compiler!\\n";
    return 0;
})";

static bool showCompilerOutput = false;

void SetTheme(bool dark)
{
    darkMode = dark;
    if (dark) ImGui::StyleColorsDark();
    else ImGui::StyleColorsLight();
}

void RenderUI()
{
    // Resizable left panel
    ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, ImGui::GetIO().DisplaySize.y - 20), ImGuiCond_Always);
    ImGui::Begin("Panel", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImGui::Text("ENGINE UI");
    ImGui::Separator();

    if (ImGui::Button("HOME", ImVec2(-1, 40))) {}
    if (ImGui::Button("FLOWCHART EDITOR", ImVec2(-1, 40))) {}
    if (ImGui::Button("CODE EDITOR", ImVec2(-1, 40))) {}

    ImGui::Separator();
    ImGui::Text("Resize:");
    ImGui::SameLine();
    ImGui::SliderFloat("##panelWidth", &panelWidth, 120.0f, 400.0f);
    ImGui::End();

    // Top bar
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 20), ImGuiCond_Always);
    ImGui::Begin("TopBar", NULL,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

    static bool fileOpen = false;
    ImGui::Text(" FILE ");
    ImGui::SameLine();
    ImGui::Text(" SETTINGS ");

    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(0)) {
        darkMode = !darkMode;
        SetTheme(darkMode);
    }

    ImGui::End();

    // Main area
    ImGui::SetNextWindowPos(ImVec2(panelWidth, 20), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - panelWidth, ImGui::GetIO().DisplaySize.y - 20), ImGuiCond_Always);
    ImGui::Begin("MainArea", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

    ImGui::Text("C++ Code Editor");
    ImGui::Separator();
    ImGui::InputTextMultiline("##code", codeBuffer.data(), codeBuffer.capacity(),
        ImVec2(-1, 300), ImGuiInputTextFlags_AllowTabInput);

    if (ImGui::Button("Compile & Run")) {
        RunCompiler(codeBuffer);
        showCompilerOutput = true;
    }

    if (showCompilerOutput) {
        ImGui::Separator();
        ImGui::TextWrapped("%s", compilerOutput.c_str());
    }

    ImGui::End();
}

// ui.cpp
#include "ui.h"
#include "compiler_interface.h"

#include "imgui.h"
#include <vector>
#include <string>
#include <algorithm>
#include <fstream>
#include "json.hpp"

// Simple resizable left panel width saved in-memory
static float panelWidth = 240.0f;
static bool fileMenuOpen = false;
static bool settingsOpen = false;
static bool darkTheme = true;

// Code buffer (ImGui needs char*)
struct CodeBuffer {
    std::vector<char> buf;
    CodeBuffer() { load_from_file("code.txt"); }
    void load_from_file(const char* path) {
        std::ifstream in(path);
        if (!in.is_open()) {
            std::string def = "// Write your code here...\n";
            buf.assign(def.begin(), def.end());
            buf.push_back('\0');
            return;
        }
        std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
        buf.assign(s.begin(), s.end());
        buf.push_back('\0');
    }
    void save_to_file(const char* path) {
        std::ofstream out(path);
        if (!out.is_open()) return;
        out.write(buf.data(), buf.size() ? (buf.size()-1) : 0);
    }
    std::string str() const { return std::string(buf.data()); }
};
static CodeBuffer codeBuf;

// Flow node
struct FlowNode { std::string name; ImVec2 pos; };
static std::vector<FlowNode> nodes;

static void DrawTopBar();
static void DrawLeftPanel();
static void DrawMainArea();

void SetTheme(bool dark) {
    darkTheme = dark;
    if (dark) ImGui::StyleColorsDark();
    else ImGui::StyleColorsLight();
}

void RenderUI() {
    // Layout: top bar fixed
    DrawTopBar();

    // Left resizable panel
    DrawLeftPanel();

    // Main area (code editor / flowchart)
    DrawMainArea();
}

static void DrawTopBar() {
    ImGui::SetNextWindowPos(ImVec2(0,0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, 28));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings;
    ImGui::Begin("TopBar", NULL, flags);
    ImGui::SameLine(6);
    if (ImGui::Button("FILE")) {
        fileMenuOpen = !fileMenuOpen;
    }
    ImGui::SameLine();
    if (ImGui::Button("SETTINGS")) {
        settingsOpen = !settingsOpen;
    }
    ImGui::SameLine();
    ImGui::Text("  Engine-UI");

    // File dropdown anchored under top bar, sticking under FILE button
    ImVec2 fileBtnPos = ImGui::GetItemRectMin();
    if (fileMenuOpen) {
        ImGui::OpenPopup("FileDropdown");
        ImGui::SetNextWindowPos(ImVec2(fileBtnPos.x, 28));
        if (ImGui::BeginPopup("FileDropdown")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) { codeBuf.buf.assign({'/','/','n','e','w','\n','\0'}); }
            if (ImGui::MenuItem("Open", "Ctrl+O")) {
                // simple open: reload code.txt
                codeBuf.load_from_file("code.txt");
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                codeBuf.save_to_file("code.txt");
            }
            if (ImGui::MenuItem("Save As", "Ctrl+Shift+S")) {
                // Save as same code.txt (placeholder)
                codeBuf.save_to_file("code.txt");
            }
            ImGui::EndPopup();
        }
    } else {
        ImGui::CloseCurrentPopup();
    }

    ImGui::End();

    // Settings window
    if (settingsOpen) {
        ImGui::SetNextWindowPos(ImVec2(200, 100), ImGuiCond_FirstUseEver);
        ImGui::Begin("Settings", &settingsOpen);
        ImGui::Text("Preferences");
        bool dark = darkTheme;
        if (ImGui::Checkbox("Dark Mode", &dark)) {
            SetTheme(dark);
        }
        ImGui::Separator();
        ImGui::Text("MinGW (g++) Path:");
        char bufpath[512] = {0};
        std::string current = GetSettings().mingw_path;
        strncpy(bufpath, current.c_str(), sizeof(bufpath)-1);
        ImGui::InputText("g++.exe path", bufpath, sizeof(bufpath));
        if (ImGui::Button("Set Path")) {
            GetSettings().mingw_path = std::string(bufpath);
            SaveSettings(); // write immediately
        }
        ImGui::End();
    }
}

static void DrawLeftPanel() {
    // Panel area
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;
    ImGui::SetNextWindowPos(ImVec2(0,28));
    ImGui::SetNextWindowSize(ImVec2(panelWidth, ImGui::GetIO().DisplaySize.y - 28));
    ImGui::Begin("LeftPanel", NULL, flags);
    if (ImGui::BeginDragDropTarget()) ImGui::EndDragDropTarget();

    if (ImGui::Button("HOME", ImVec2(-1, 0))) { /* switch screens handled in main area */ }
    if (ImGui::Button("FLOWCHART EDITOR", ImVec2(-1, 0))) { /* handled in main area */ }
    if (ImGui::Button("CODE EDITOR", ImVec2(-1, 0))) { /* handled in main area */ }

    ImGui::Dummy(ImVec2(0,10));
    ImGui::TextWrapped("Left panel (drag the right border to resize)");
    ImGui::End();

    // Resize handle: simple invisible drag area
    ImGui::SetNextWindowPos(ImVec2(panelWidth-4, 28));
    ImGui::SetNextWindowSize(ImVec2(8, ImGui::GetIO().DisplaySize.y - 28));
    ImGui::Begin("ResizeHandle", NULL, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoInputs);
    ImGui::InvisibleButton("##resize", ImVec2(8, ImGui::GetIO().DisplaySize.y - 28));
    if (ImGui::IsItemActive()) {
        float dx = ImGui::GetIO().MouseDelta.x;
        panelWidth += dx;
        if (panelWidth < 120.0f) panelWidth = 120.0f;
        if (panelWidth > 600.0f) panelWidth = 600.0f;
    }
    ImGui::End();
}

static enum MainMode {MM_Home, MM_Code, MM_Flow} mainMode = MM_Home;

static void DrawHomeScreen() {
    ImGui::Begin("Home", NULL, ImGuiWindowFlags_NoCollapse);
    ImGui::Text("Welcome to Engine-UI");
    if (ImGui::Button("Open Code Editor", ImVec2(200,0))) mainMode = MM_Code;
    if (ImGui::Button("Open Flowchart Editor", ImVec2(200,0))) mainMode = MM_Flow;
    ImGui::End();
}

static void DrawCodeEditor() {
    ImGui::Begin("Code Editor", NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar);
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("Build")) {
            if (ImGui::MenuItem("Compile")) RunCompilerAndCapture("code.txt");
            if (ImGui::MenuItem("Clear Output")) ClearCompilerOutput();
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    ImGui::SameLine();
    if (ImGui::Button("Save")) codeBuf.save_to_file("code.txt");
    ImGui::SameLine();
    if (ImGui::Button("Compile")) RunCompilerAndCapture("code.txt");

    // InputTextMultiline requires char* buffer
    ImGui::Text("Source:");
    ImGui::BeginChild("##codechild", ImVec2(-1, 300), true);
    ImGui::PushTextWrapPos(0.0f);
    ImGui::InputTextMultiline("##code", codeBuf.buf.data(), codeBuf.buf.size(),
                              ImVec2(-1, -1),
                              ImGuiInputTextFlags_AllowTabInput);
    ImGui::PopTextWrapPos();
    ImGui::EndChild();

    // Compiler output area
    ImGui::Separator();
    ImGui::Text("Compiler Output:");
    ShowCompilerOutputUI(); // from compiler_interface
    ImGui::End();
}

static void DrawFlowchartEditor() {
    ImGui::Begin("Flowchart Editor", NULL, ImGuiWindowFlags_NoCollapse);

    static char newName[64] = "";
    ImGui::InputText("Node Name", newName, sizeof(newName));
    ImGui::SameLine();
    if (ImGui::Button("Add Node")) {
        nodes.push_back({ std::string(newName), ImVec2(300, 200) });
        newName[0] = '\0';
    }
    ImGui::SameLine();
    if (ImGui::Button("Save Flowchart")) {
        // save simple JSON
        nlohmann::json j = nlohmann::json::array();
        for (auto &n : nodes) {
            j.push_back({ {"name", n.name}, {"x", n.pos.x}, {"y", n.pos.y} });
        }
        std::ofstream out("flowchart.json");
        out << j.dump(2);
    }

    ImGui::Separator();

    // Canvas area (pan/zoom not implemented - simple)
    ImGui::BeginChild("canvas", ImVec2(-1, -1), true);
    for (size_t i=0;i<nodes.size();++i) {
        ImGui::SetCursorScreenPos(nodes[i].pos);
        ImGui::Button(("##node"+std::to_string(i)).c_str(), ImVec2(140,60));
        ImGui::SetItemAllowOverlap();
        if (ImGui::IsItemActive() && ImGui::IsMouseDragging(0)) {
            ImVec2 delta = ImGui::GetIO().MouseDelta;
            nodes[i].pos.x += delta.x;
            nodes[i].pos.y += delta.y;
        }
        // Draw node label separately overlay
        ImGui::SameLine();
        ImGui::SetCursorScreenPos(ImVec2(nodes[i].pos.x + 8, nodes[i].pos.y + 8));
        ImGui::Text("%s", nodes[i].name.c_str());
    }
    ImGui::EndChild();
    ImGui::End();
}

static void DrawMainArea() {
    float left = panelWidth;
    ImGui::SetNextWindowPos(ImVec2(left, 28));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - left, ImGui::GetIO().DisplaySize.y - 28));
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar;
    ImGui::Begin("MainArea", NULL, flags);

    // Small top tabs
    if (ImGui::Button("Home")) mainMode = MM_Home;
    ImGui::SameLine();
    if (ImGui::Button("Code")) mainMode = MM_Code;
    ImGui::SameLine();
    if (ImGui::Button("Flowchart")) mainMode = MM_Flow;
    ImGui::Separator();

    switch (mainMode) {
        case MM_Home: DrawHomeScreen(); break;
        case MM_Code: DrawCodeEditor(); break;
        case MM_Flow: DrawFlowchartEditor(); break;
    }

    ImGui::End();
}

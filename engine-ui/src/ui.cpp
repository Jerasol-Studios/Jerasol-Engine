#include "ui.h"
#include "imgui.h"
#include "imgui_internal.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

static EditorPage currentPage = EditorPage::Home;
static bool darkTheme = true;

// code buffer as dynamic char array to satisfy ImGui InputTextMultiline
static std::vector<char> codeBuffer;
static std::string currentFilePath;
static bool fileModalOpen = false;
static char filePathInput[1024] = "";

static void ensure_codebuf_size(size_t minSize) {
    if (codeBuffer.size() < minSize+1) {
        codeBuffer.resize(minSize+1);
    }
}

static void open_file_into_buffer(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    if (!f.is_open()) return;
    std::ostringstream ss;
    ss << f.rdbuf();
    std::string s = ss.str();
    ensure_codebuf_size(s.size());
    memcpy(codeBuffer.data(), s.data(), s.size());
    codeBuffer[s.size()] = '\0';
    currentFilePath = path;
}

static void save_buffer_to_file(const std::string& path) {
    std::ofstream f(path, std::ios::binary);
    if (!f.is_open()) return;
    f.write(codeBuffer.data(), strlen(codeBuffer.data()));
    currentFilePath = path;
}

static void FileOpenModal() {
    if (!fileModalOpen) return;
    ImGui::OpenPopup("Open File###openfile");
    fileModalOpen = false;
}

static void DoFileModal() {
    if (ImGui::BeginPopupModal("Open File###openfile", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Enter file path:");
        ImGui::InputText("##path", filePathInput, IM_ARRAYSIZE(filePathInput));
        if (ImGui::Button("Open")) {
            open_file_into_buffer(std::string(filePathInput));
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }
        ImGui::EndPopup();
    }
}

static void DrawToolbar(FlowchartEditor& flowEditor, CompilerInterface& compiler) {
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::BeginMainMenuBar()) {
        // FILE menu anchored to top bar
        if (ImGui::BeginMenu("FILE")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                // New: clear buffer and unset path
                codeBuffer.assign(1, '\0');
                currentFilePath.clear();
                currentPage = EditorPage::TextEditor;
            }
            if (ImGui::MenuItem("Open...", "Ctrl+O")) {
                fileModalOpen = true;
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                if (currentFilePath.empty()) {
                    ImGui::OpenPopup("Save As");
                } else {
                    save_buffer_to_file(currentFilePath);
                }
            }
            if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S")) {
                ImGui::OpenPopup("Save As");
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("SETTINGS")) {
            if (ImGui::MenuItem(darkTheme ? "Light Mode" : "Dark Mode")) {
                darkTheme = !darkTheme;
                SetTheme(darkTheme);
            }
            ImGui::EndMenu();
        }

        // quick page buttons on top bar
        ImGui::Separator();
        if (ImGui::Button("HOME")) currentPage = EditorPage::Home;
        ImGui::SameLine();
        if (ImGui::Button("FLOWCHART")) currentPage = EditorPage::Flowchart;
        ImGui::SameLine();
        if (ImGui::Button("CODE")) currentPage = EditorPage::TextEditor;

        ImGui::EndMainMenuBar();
    }

    // Save As popup
    if (ImGui::BeginPopupModal("Save As", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        static char savePath[1024] = "";
        ImGui::InputText("Save path", savePath, IM_ARRAYSIZE(savePath));
        if (ImGui::Button("Save")) {
            save_buffer_to_file(savePath);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // keyboard shortcuts
    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_N)) {
        codeBuffer.assign(1,'\0');
        currentFilePath.clear();
        currentPage = EditorPage::TextEditor;
    }
    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_O)) {
        fileModalOpen = true;
    }
    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S)) {
        if (ImGui::GetIO().KeyShift) {
            ImGui::OpenPopup("Save As");
        } else {
            if (!currentFilePath.empty()) save_buffer_to_file(currentFilePath);
            else ImGui::OpenPopup("Save As");
        }
    }

    // file modal open
    FileOpenModal();
    DoFileModal();
}

static void DrawHome() {
    ImGui::Begin("Home");
    ImGui::Text("Welcome to the Engine UI (Phase 3)");
    ImGui::Separator();
    ImGui::TextWrapped("Use the top menu or buttons to switch between editors, create files, and compile.");
    ImGui::End();
}

static void DrawTextEditor(CompilerInterface& compiler) {
    ImGui::Begin("Code Editor", nullptr, ImGuiWindowFlags_NoCollapse);

    // ensure buffer exists
    ensure_codebuf_size(1024);

    // label shows current path
    if (currentFilePath.empty()) ImGui::Text("File: (unsaved)");
    else ImGui::Text("File: %s", currentFilePath.c_str());

    ImGui::Separator();

    // InputTextMultiline - uses char*
    ImGui::InputTextMultiline("##code", codeBuffer.data(), codeBuffer.size(),
        ImVec2(-1, ImGui::GetTextLineHeight() * 20),
        ImGuiInputTextFlags_AllowTabInput);

    ImGui::Spacing();
    if (ImGui::Button("Compile & Build")) {
        // write temporary source if buffer changed
        std::string tmpSource = "temp_build.cpp";
        save_buffer_to_file(tmpSource);
        // target exe
        std::string exe = "output_program.exe";
        compiler.CompileCode(tmpSource, exe);
    }
    ImGui::SameLine();
    if (ImGui::Button("Run")) {
        compiler.RunExecutable("output_program.exe", ".");
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear Output")) compiler.ClearOutput();

    ImGui::Separator();
    compiler.RenderOutput();

    ImGui::End();
}

void RenderUI(FlowchartEditor& flowEditor, CompilerInterface& compiler) {
    DrawToolbar(flowEditor, compiler);

    // Left fixed panel (resizable)
    static float panelWidth = 220.0f;
    ImGuiWindowFlags leftWinFlags = ImGuiWindowFlags_NoCollapse;
    ImGui::SetNextWindowPos(ImVec2(0, ImGui::GetFrameHeight()), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(panelWidth, ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight()));
    ImGui::Begin("Panel", nullptr, leftWinFlags);
    if (ImGui::Selectable("HOME", currentPage == EditorPage::Home)) currentPage = EditorPage::Home;
    if (ImGui::Selectable("FLOWCHART", currentPage == EditorPage::Flowchart)) currentPage = EditorPage::Flowchart;
    if (ImGui::Selectable("CODE EDITOR", currentPage == EditorPage::TextEditor)) currentPage = EditorPage::TextEditor;
    ImGui::End();

    // Allow resizing (simple approach)
    ImGui::SetNextWindowPos(ImVec2(panelWidth - 4, ImGui::GetFrameHeight()), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(8, ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight()));
    ImGui::Begin("##panel_resizer", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        panelWidth += ImGui::GetIO().MouseDelta.x;
        if (panelWidth < 120.0f) panelWidth = 120.0f;
        if (panelWidth > 600.0f) panelWidth = 600.0f;
    }
    ImGui::End();

    // Main area (rest of screen)
    ImGui::SetNextWindowPos(ImVec2(panelWidth, ImGui::GetFrameHeight()), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x - panelWidth, ImGui::GetIO().DisplaySize.y - ImGui::GetFrameHeight()));
    ImGui::Begin("MainArea", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse);

    switch (currentPage) {
        case EditorPage::Home: DrawHome(); break;
        case EditorPage::TextEditor: DrawTextEditor(compiler); break;
        case EditorPage::Flowchart: flowEditor.Render(); break;
    }

    ImGui::End();
}

void SetTheme(bool dark) {
    ImGuiStyle& style = ImGui::GetStyle();
    if (dark) ImGui::StyleColorsDark();
    else ImGui::StyleColorsLight();

    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.Colors[ImGuiCol_Header] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
}

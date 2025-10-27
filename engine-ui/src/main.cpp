#include "ui.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"
#include "SDL.h"
#include "SDL_main.h"
#include <string>
#include <vector>
#include <fstream>
#include <stdio.h>
#include "json.hpp" // thirdparty/json.hpp

// (rest of your previous main.cpp code)


using json = nlohmann::json;

enum AppState {
    HOME_MENU,
    CODE_EDITOR,
    FLOWCHART_EDITOR
};

struct FlowNode {
    std::string name;
    ImVec2 position;
};

// Serialize/deserialize for JSON
void to_json(json& j, const FlowNode& node) {
    j = json{{"name", node.name}, {"x", node.position.x}, {"y", node.position.y}};
}
void from_json(const json& j, FlowNode& node) {
    node.name = j.at("name").get<std::string>();
    node.position = ImVec2(j.at("x").get<float>(), j.at("y").get<float>());
}

int main(int, char**) {
    // SDL Initialization
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0) {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Window* window = SDL_CreateWindow("Engine-UI",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    AppState state = HOME_MENU;
    std::string codeText;

    // Load code if exists
    std::ifstream codeFile("code.txt");
    if (codeFile.is_open()) {
        codeText.assign((std::istreambuf_iterator<char>(codeFile)),
                         std::istreambuf_iterator<char>());
        codeFile.close();
    } else {
        codeText = "// Write your code here...\n";
    }

    std::vector<FlowNode> nodes;
    // Load flowchart nodes
    std::ifstream nodeFile("flowchart.json");
    if (nodeFile.is_open()) {
        json j;
        nodeFile >> j;
        nodes = j.get<std::vector<FlowNode>>();
    }

    bool running = true;
    SDL_Event event;
    while (running) {
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) running = false;
        }

        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        // --- render UI ---
        RenderUI();  // <--- this calls the interface we made

        // Rendering
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 45, 45, 48, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
    }

    // Auto-save on exit
    {
        std::ofstream out("code.txt"); out << codeText;
        json j = nodes;
        std::ofstream outNodes("flowchart.json"); outNodes << j.dump(4);
    }

    // Cleanup
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

#include <SDL.h>

int main(int argc, char* argv[]); // forward declaration

int WinMain(int argc, char* argv[])
{
    return main(argc, argv);
}

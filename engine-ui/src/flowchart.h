#pragma once
#include "imgui.h"
#include <vector>
#include <string>

struct FlowNode {
    int id;
    std::string name;
    ImVec2 pos;
    ImVec2 size;
};

struct FlowConnection {
    int from;
    int to;
};

struct FlowchartEditor {
    FlowchartEditor();
    void Render();

    void AddNode(const char* name, ImVec2 pos = ImVec2(50,50));
    void RemoveNode(int id);
    void SaveToFile(const std::string& path);
    void LoadFromFile(const std::string& path);

    std::vector<FlowNode> nodes;
    std::vector<FlowConnection> connections;

private:
    int nextId;
    int selectedNode;
    int linkStartNode;
    bool draggingNode;
};

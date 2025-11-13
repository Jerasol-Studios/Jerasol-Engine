#include "flowchart.h"
#include "json.hpp"
#include <fstream>

using json = nlohmann::json;

FlowchartEditor::FlowchartEditor()
    : nextId(1), selectedNode(-1), linkStartNode(-1), draggingNode(false)
{
}

void FlowchartEditor::AddNode(const char* name, ImVec2 pos) {
    FlowNode n;
    n.id = nextId++;
    n.name = name ? name : "Node";
    n.pos = pos;
    n.size = ImVec2(120, 60);
    nodes.push_back(n);
}

void FlowchartEditor::RemoveNode(int id) {
    for (size_t i=0;i<nodes.size();++i) {
        if (nodes[i].id == id) { nodes.erase(nodes.begin()+i); break; }
    }
    // remove connections
    connections.erase(std::remove_if(connections.begin(), connections.end(),
        [id](const FlowConnection& c){ return c.from==id || c.to==id; }), connections.end());
}

void FlowchartEditor::SaveToFile(const std::string& path) {
    json j;
    j["nodes"] = json::array();
    j["connections"] = json::array();
    for (auto &n : nodes) {
        j["nodes"].push_back({{"id", n.id}, {"name", n.name}, {"x", n.pos.x}, {"y", n.pos.y}});
    }
    for (auto &c : connections) {
        j["connections"].push_back({{"from", c.from}, {"to", c.to}});
    }
    std::ofstream f(path);
    if (f.is_open()) {
        f << j.dump(4);
    }
}

void FlowchartEditor::LoadFromFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.is_open()) return;
    json j;
    f >> j;
    nodes.clear();
    connections.clear();
    for (auto &jn : j["nodes"]) {
        FlowNode n;
        n.id = jn.value("id", nextId++);
        n.name = jn.value("name", std::string("Node"));
        n.pos = ImVec2(jn.value("x", 50.0f), jn.value("y", 50.0f));
        n.size = ImVec2(120,60);
        nodes.push_back(n);
    }
    for (auto &jc : j["connections"]) {
        FlowConnection c;
        c.from = jc.value("from", 0);
        c.to = jc.value("to", 0);
        connections.push_back(c);
    }
}

void FlowchartEditor::Render() {
    // Simple canvas: we draw nodes as draggable buttons and lines as connections
    ImGui::Begin("Flowchart Editor", nullptr, ImGuiWindowFlags_NoCollapse);

    // Toolbar in the flowchart window
    if (ImGui::Button("Add Node")) { AddNode("Node", ImVec2(100,100)); }
    ImGui::SameLine();
    if (ImGui::Button("Save")) { SaveToFile("flowchart.json"); }
    ImGui::SameLine();
    if (ImGui::Button("Load")) { LoadFromFile("flowchart.json"); }
    ImGui::SameLine();
    if (ImGui::Button("Clear")) { nodes.clear(); connections.clear(); }

    ImGui::Separator();

    ImVec2 canvas_p0 = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
    ImVec2 canvas_sz = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y - 10);
    if (canvas_sz.x < 50.0f) canvas_sz.x = 50.0f;
    if (canvas_sz.y < 50.0f) canvas_sz.y = 50.0f;
    ImVec2 canvas_p1 = ImVec2(canvas_p0.x + canvas_sz.x, canvas_p0.y + canvas_sz.y);

    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50,50,55,255));
    draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(120,120,120,255));

    // Pan/zoom could be added here; keep simple.

    // Draw connections first
    for (auto &c : connections) {
        FlowNode* a = nullptr;
        FlowNode* b = nullptr;
        for (auto &n : nodes) {
            if (n.id == c.from) a = &n;
            if (n.id == c.to) b = &n;
        }
        if (!a || !b) continue;
        ImVec2 p1 = ImVec2(a->pos.x + a->size.x, a->pos.y + a->size.y * 0.5f);
        ImVec2 p2 = ImVec2(b->pos.x, b->pos.y + b->size.y * 0.5f);
        ImVec2 cp1 = ImVec2(p1.x + 50, p1.y);
        ImVec2 cp2 = ImVec2(p2.x - 50, p2.y);
        draw_list->AddBezierCubic(p1, cp1, cp2, p2, IM_COL32(200,200,100,255), 3.0f);
        // arrow head
        ImVec2 dir = ImVec2(p2.x - cp2.x, p2.y - cp2.y);
    }

    // Handle clicks on canvas background to create nodes
    ImGui::InvisibleButton("canvas", canvas_sz, ImGuiButtonFlags_MouseButtonLeft);
    const bool is_hovered = ImGui::IsItemHovered(); // hover canvas

    // Node interactions
    for (size_t i = 0; i < nodes.size(); ++i) {
        FlowNode &n = nodes[i];

        // node rectangle in screen coords
        ImVec2 node_p0 = ImVec2(canvas_p0.x + n.pos.x, canvas_p0.y + n.pos.y);
        ImVec2 node_p1 = ImVec2(node_p0.x + n.size.x, node_p0.y + n.size.y);

        // Draw node background and title
        draw_list->AddRectFilled(node_p0, node_p1, IM_COL32(70,70,90,255), 6.0f);
        draw_list->AddRect(node_p0, node_p1, IM_COL32(120,120,140,255), 6.0f);
        draw_list->AddText(ImVec2(node_p0.x + 8, node_p0.y + 6), IM_COL32(255,255,255,255), n.name.c_str());

        // detect dragging - we'll use io.MouseDelta when the user clicks the node area
        ImVec2 mp = io.MousePos;
        bool inside = (mp.x >= node_p0.x && mp.x <= node_p1.x && mp.y >= node_p0.y && mp.y <= node_p1.y);

        if (inside && ImGui::IsMouseClicked(0)) {
            selectedNode = n.id;
            draggingNode = true;
        }
        if (draggingNode && ImGui::IsMouseDown(0) && selectedNode == n.id) {
            n.pos.x += io.MouseDelta.x;
            n.pos.y += io.MouseDelta.y;
            // clamp within canvas
            if (n.pos.x < 0) n.pos.x = 0;
            if (n.pos.y < 0) n.pos.y = 0;
            if (n.pos.x + n.size.x > canvas_sz.x) n.pos.x = canvas_sz.x - n.size.x;
            if (n.pos.y + n.size.y > canvas_sz.y) n.pos.y = canvas_sz.y - n.size.y;
        }
        if (ImGui::IsMouseReleased(0)) {
            draggingNode = false;
        }

        // Right-click menu per node
        if (inside && ImGui::IsMouseClicked(1)) {
            ImGui::OpenPopup(("node_popup_" + std::to_string(n.id)).c_str());
        }
        if (ImGui::BeginPopup(("node_popup_" + std::to_string(n.id)).c_str())) {
            if (ImGui::MenuItem("Start link")) {
                linkStartNode = n.id;
            }
            if (ImGui::MenuItem("Delete")) {
                RemoveNode(n.id);
                ImGui::EndPopup();
                break; // nodes changed
            }
            ImGui::EndPopup();
        }

        // if linking and clicking another node, create connection
        if (linkStartNode != -1 && inside && ImGui::IsMouseClicked(0) && linkStartNode != n.id) {
            FlowConnection c; c.from = linkStartNode; c.to = n.id;
            connections.push_back(c);
            linkStartNode = -1;
        }
    }

    ImGui::End();
}

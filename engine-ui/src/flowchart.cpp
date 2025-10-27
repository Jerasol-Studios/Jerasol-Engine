#include "flowchart.h"
#include "json.hpp"  // Make sure -Ithirdparty is in compiler flags

using json = nlohmann::json;

// Example function to create JSON from a FlowchartNode
json nodeToJson(const FlowchartNode& node) {
    json j;
    j["name"] = node.name;
    j["id"] = node.id;
    return j;
}


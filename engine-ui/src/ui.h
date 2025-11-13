#pragma once
#include "flowchart.h"
#include "compiler_interface.h"
#include "imgui.h"

enum class EditorPage
{
    Home,
    TextEditor,
    Flowchart
};

void RenderUI(FlowchartEditor& flowEditor, CompilerInterface& compiler);
void SetTheme(bool dark);

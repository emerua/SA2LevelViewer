#include "MenuManager.h"

MenuManager::MenuManager(GLFWwindow* window) : context(ImGui::CreateContext()), io(ImGui::GetIO()) {
    IMGUI_CHECKVERSION();
    (void)io;


    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 400");
}

void MenuManager::InitRender() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

bool MenuManager::CreateViewWindow() {
    bool shouldLoadNewLevel = false;

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_::ImGuiCond_Once);
    ImGui::Begin("Options", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);

    ImGui::Text("File Options");
    ImGui::Indent(16.0f);
    ImGui::Text("Load level objects");
    ImGui::SameLine();
    shouldLoadNewLevel = ImGui::Button("Open");
    ImGui::Unindent(16.0f);

    ImGui::Separator();

    ImGui::Text("View Options");
    ImGui::Indent(16.0f);
    ImGui::Checkbox("Lock Camera", &lockCamera);
    ImGui::Checkbox("View Stage", &displayStage);
    ImGui::Checkbox("View Collision", &displayStageCollision);
    ImGui::Checkbox("View Killplanes", &displayStageKillplanes);
    ImGui::Checkbox("View Background", &displayStageSky);
    ImGui::Checkbox("View Camera Triggers", &displayCameraTriggers);
    ImGui::Checkbox("View Loopspeed Triggers", &displayLoopspeedTriggers);
    ImGui::Checkbox("Backface Culling", &renderWithCulling);
    ImGui::Unindent(16.0f);

    ImGui::Separator();

    ImGui::Text("Sa2 Options");
    ImGui::Indent(16.0f);
    ImGui::Checkbox("Load level objects automatically", &autoLoadObjects);
    ImGui::Checkbox("Follow SA2 in RealTime", &isFollowRealTime);
    ImGui::Indent(16.0f);
    ImGui::BeginDisabled(!isFollowRealTime);
    ImGui::Checkbox("No Follow Camera", &gameIsFollowingSA2NoCam);
    ImGui::EndDisabled();

    ImGui::SetWindowSize("Options", ImVec2(0, 0), ImGuiCond_::ImGuiCond_Once);
    ImGui::End();

    return shouldLoadNewLevel;
}

void MenuManager::CreateHelpWindow(std::string version) {
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_::ImGuiCond_Once);
    ImGui::Begin("Help", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);

    ImGui::Text((("Version " + version) + "\nThis project is forked from TurtleMan64/SA2LevelEditor\n\n"
        "Load the U and S setfile in and the rest of the stage models will load automatically.\n"
        "Controls:\n"
        "    Mouse scroll to move camera forward/backward\n"
        "    Mouse left click + mouse move to rotate camera\n"
        "    Mouse left click + WASDQE to move camera\n"
        "    * All controls are available while \"Lock Camera\" setting be unchecking only.\n\n"
        "----------\n\n"
        R"(imgui

The MIT License (MIT)

Copyright (c) 2014-2022 Omar Cornut

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.)").c_str());

    ImGui::SetWindowSize("Help", ImVec2(0, 0), ImGuiCond_::ImGuiCond_Once);
    ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    ImVec2 vMax = ImGui::GetWindowContentRegionMax();
    ImGuiStyle style = ImGui::GetStyle();
    float contentWidth = vMax.x - vMin.x + style.WindowPadding.x * 2;
    ImGui::SetWindowPos("Help", ImVec2(ImGui::GetIO().DisplaySize.x - contentWidth - 10, 10), ImGuiCond_::ImGuiCond_Once);
    ImGui::End();
}

void MenuManager::Render() {
    // Rendering
    ImGui::Render();

    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

bool MenuManager::isMouseCaptured()
{
    return io.WantCaptureMouse;
}

MenuManager::~MenuManager()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(context);
}

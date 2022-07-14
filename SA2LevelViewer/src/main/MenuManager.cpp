#include "MenuManager.h"
#include "..\resource\resource.h"
#include <Windows.h>
#include <fstream>
#include <sstream>

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
        ).c_str());

    openLicenseWindow = ImGui::Button("License") || openLicenseWindow;
    ImGui::SetWindowSize("Help", ImVec2(0, 0), ImGuiCond_::ImGuiCond_Once);
    ImVec2 vMin = ImGui::GetWindowContentRegionMin();
    ImVec2 vMax = ImGui::GetWindowContentRegionMax();
    ImGuiStyle style = ImGui::GetStyle();
    float contentWidth = vMax.x - vMin.x + style.WindowPadding.x * 2;
    ImGui::SetWindowPos("Help", ImVec2(ImGui::GetIO().DisplaySize.x - contentWidth - 10, 10), ImGuiCond_::ImGuiCond_Once);
    ImGui::End();

    if (openLicenseWindow) {
        CreateLicenseWindow();
    }
}

void MenuManager::CreateLicenseWindow()
{
    HRSRC resInfo = FindResource(NULL, MAKEINTRESOURCE(LICENSE_INFO), "TXT");
    if (resInfo == NULL)
    {
        OutputDebugString("resInfo is NULL");
        return;
    }

    HGLOBAL handle = LoadResource(NULL, resInfo);
    if (handle == NULL)
    {
        OutputDebugString("handle is NULL");
        return;
    }

    DWORD datasize = SizeofResource(NULL, resInfo);
    LPVOID data = LockResource(handle);

    if (datasize == 0 || data == NULL)
    {
        OutputDebugString("data is NULL");
        return;
    }

    std::stringstream ss{ std::string((char*)data, datasize) };
    std::string licenseInfoStr = ss.str();
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::OpenPopup("License");
    if (ImGui::BeginPopupModal("License", &openLicenseWindow, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize + ImGuiWindowFlags_::ImGuiWindowFlags_NoMove + ImGuiWindowFlags_::ImGuiWindowFlags_NoCollapse + ImGuiWindowFlags_::ImGuiWindowFlags_HorizontalScrollbar))
    {
        ImGui::TextUnformatted(licenseInfoStr.c_str());
        float width = fmaxf(io.DisplaySize.x * 0.8f, 512.0f);
        float height = fmaxf(io.DisplaySize.y * 0.8, 288.0f);
        ImGui::SetWindowSize("License", ImVec2(width, height), ImGuiCond_::ImGuiCond_Always);
        ImGui::EndPopup();
    }
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

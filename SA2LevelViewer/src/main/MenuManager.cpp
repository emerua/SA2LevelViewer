#include <Windows.h>
#include <fstream>
#include <sstream>
#include <format>
#include "MenuManager.h"
#include "..\resource\resource.h"
#include "../main/main.h"
#include "../entities/camera.h"

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

void MenuManager::CreateViewWindow() {
    ImGui::Begin("Options", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);

    collapsedFileOption = ImGui::TreeNodeEx("File Options");
    if (collapsedFileOption)
    {
        ImGui::PushItemWidth(16.0f);
        // ImGui::Indent(16.0f);
        ImGui::Text("Load level objects");
        ImGui::SameLine();
        Global::shouldLoadNewLevel = ImGui::Button("Open");
        // ImGui::Unindent(16.0f);
        ImGui::PopItemWidth();
        ImGui::TreePop();
    }

    ImGui::Separator();

    collapsedViewOption = ImGui::TreeNodeEx("View Options");
    if (collapsedViewOption)
    {
        ImGui::PushItemWidth(16.0f);
        ImGui::Checkbox("Lock Camera", &lockCamera);
        ImGui::Checkbox("View Stage", &displayStage);
        ImGui::Checkbox("View Collision", &displayStageCollision);
        ImGui::Checkbox("View Killplanes", &displayStageKillplanes);
        ImGui::Checkbox("View Background", &displayStageSky);
        ImGui::Checkbox("View Camera Triggers", &displayCameraTriggers);
        ImGui::Checkbox("View Loopspeed Triggers", &displayLoopspeedTriggers);
        ImGui::Checkbox("Backface Culling", &renderWithCulling);
        ImGui::PopItemWidth();
        ImGui::TreePop();
    }

    ImGui::Separator();

    collapsedSA2Option = ImGui::TreeNodeEx("Sa2 Options");
    if (collapsedSA2Option)
    {
        ImGui::PushItemWidth(16.0f);
        ImGui::Checkbox("Load level objects automatically", &autoLoadObjects);
        ImGui::Checkbox("Follow SA2 in RealTime", &isFollowRealTime);
        ImGui::Indent(16.0f);
        ImGui::BeginDisabled(!isFollowRealTime);
        ImGui::Checkbox("No Follow Camera", &gameIsFollowingSA2NoCam);
        ImGui::EndDisabled();
        ImGui::PopItemWidth();
        ImGui::TreePop();
    }
    ImGui::SetWindowSize("Options", ImVec2(300.f, 0), ImGuiCond_::ImGuiCond_Always);
    ImGui::SetWindowPos(ImVec2(10, 10), ImGuiCond_::ImGuiCond_FirstUseEver);
    adjustWindow("Options");
    ImGui::End();
}

void MenuManager::CreateCameraWindow() {
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_::ImGuiCond_FirstUseEver);
    ImGui::Begin("Camera Locations", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);

    ImGui::Checkbox("Show Dialog Before Save", &confirmSave);
    ImGui::Checkbox("Show Dialog Before Load", &confirmLoad);

    ImGui::Separator();
    ImGui::Text("LevelID: [%d]", Global::levelID);

    if (ImGui::BeginTable("table1", 4, ImGuiTableFlags_Borders))
    {
        ImGui::TableSetupColumn("No.", ImGuiTableColumnFlags_::ImGuiTableColumnFlags_WidthFixed, 200.0F);
        ImGui::TableSetupColumn("Save", ImGuiTableColumnFlags_::ImGuiTableColumnFlags_WidthFixed, 40.0F);
        ImGui::TableSetupColumn("Load", ImGuiTableColumnFlags_::ImGuiTableColumnFlags_WidthFixed, 40.0F);
        ImGui::TableSetupColumn("Default", ImGuiTableColumnFlags_::ImGuiTableColumnFlags_WidthFixed, 50.0F);
        ImGui::TableHeadersRow();
        for (int row = 1; row <= 5; row++)
        {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            std::string slotText = "Not Set";
            if (Global::levelID != 0 && camLocations.find(row) != camLocations.end())
            {
                auto [eye, pitch, yaw] = camLocations.at(row);
                slotText = std::format("({:.0f}, {:.0f}, {:.0f})", eye.x, eye.y, eye.z);
            }
            ImGui::Text("[%d] %s", row, slotText.c_str());

            ImGui::TableSetColumnIndex(1);
            if (ImGui::Button(std::format("Save##{}", row).c_str()))
            {
                saveCameraLocation(row);
            }

            ImGui::TableSetColumnIndex(2);
            if (ImGui::Button(std::format("Load##{}", row).c_str()))
            {
                loadCameraLocation(row);
            }

            ImGui::TableSetColumnIndex(3);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 25.0F - ImGui::GetFrameHeight() / 2);
            ImGui::Checkbox(std::format("##CheckBox{}", row).c_str(), &lockCamera);
        }
        ImGui::EndTable();
    }

    ImGui::Separator();
    ImGui::SetWindowSize("Camera Locations", ImVec2(0, 0), ImGuiCond_::ImGuiCond_Always);
    ImGui::SetWindowPos("Camera Locations", ImVec2(10, ImGui::GetIO().DisplaySize.y - ImGui::GetWindowHeight() - 10), ImGuiCond_::ImGuiCond_FirstUseEver);
    adjustWindow("Camera Locations");

    ImGui::End();
}

void MenuManager::CreateHelpWindow() {
    ImGui::SetNextWindowCollapsed(true, ImGuiCond_::ImGuiCond_FirstUseEver);
    ImGui::Begin("Help", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);

    ImGui::Text((("Version " + Global::version) + "\nThis project is forked from TurtleMan64/SA2LevelEditor\n\n"
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
    ImGui::SetWindowPos("Help", ImVec2(ImGui::GetIO().DisplaySize.x - ImGui::GetWindowWidth() - 10, 10), ImGuiCond_::ImGuiCond_FirstUseEver);
    adjustWindow("Help");
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
        float height = fmaxf(io.DisplaySize.y * 0.8f, 288.0f);
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

bool MenuManager::saveCameraLocation(int slotID)
{
    if (Global::levelID == 0)
    {
        return false;
    }

    if (confirmSave)
    {
        int response = MessageBox(NULL,
            std::format("Save Camera Location #{}?", slotID).c_str(),
            "Save Camera Location",
            MB_YESNO);
        if (response != IDYES)
        {
            return false;
        }
    }
    camLocations[slotID] = { Global::gameCamera->eye, Global::gameCamera->pitch, Global::gameCamera->yaw };

    return true;
}

bool MenuManager::loadCameraLocation(int slotID)
{
    if (Global::levelID == 0 || camLocations.find(slotID) == camLocations.end())
    {
        return false;
    }

    if (confirmLoad)
    {
        int response = MessageBox(NULL,
            std::format("Load Camera Location #{}?", slotID).c_str(),
            "Load Camera Location",
            MB_YESNO);
        if (response != IDYES)
        {
            return false;
        }
    }

    auto [eye, pitch, yaw] = camLocations.at(slotID);
    Global::gameCamera->eye = eye;
    Global::gameCamera->pitch = pitch;
    Global::gameCamera->yaw = yaw;

    return true;
}

MenuManager::~MenuManager()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(context);
}

ImVec2 MenuManager::adjustWindow(const char* const name)
{
    ImVec2 currentPos = ImGui::GetWindowPos();
    float x = fminf(fmaxf(10, currentPos.x), io.DisplaySize.x - ImGui::GetWindowWidth() - 10);
    float y = fminf(fmaxf(10, currentPos.y), io.DisplaySize.y - ImGui::GetWindowHeight() - 10);
    ImVec2 newPos = ImVec2(x, y);
    ImGui::SetWindowPos(name, newPos, ImGuiCond_::ImGuiCond_Always);
    return newPos;
}

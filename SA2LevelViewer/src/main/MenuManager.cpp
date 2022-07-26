#include <Windows.h>
#include <fstream>
#include <sstream>
#include <format>

#include <json/json.hpp>

#include "MenuManager.h"
#include "../resource/resource.h"
#include "../main/main.h"
#include "../entities/camera.h"
#include "../toolbox/vector.h"

using json = nlohmann::json;

MenuManager::MenuManager(GLFWwindow* window) : context(ImGui::CreateContext()), io(ImGui::GetIO()) {
    IMGUI_CHECKVERSION();
    (void)io;
    io.IniFilename = "Settings/imgui.ini";

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 400");

    loadSettings();
}

void MenuManager::InitRender() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void MenuManager::CreateViewWindow() {
    ImGui::Begin("Options", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoResize);
    
    ImGui::SetNextItemOpen(collapsedFileOption);
    collapsedFileOption = ImGui::TreeNodeEx("File Options");
    if (collapsedFileOption)
    {
        ImGui::PushItemWidth(16.0f);
        // ImGui::Indent(16.0f);
        ImGui::BeginDisabled(autoLoadObjects);
        ImGui::Text("Load level objects");
        ImGui::SameLine();
        Global::shouldLoadNewLevel = ImGui::Button("Open");
        ImGui::EndDisabled();
        // ImGui::Unindent(16.0f);
        ImGui::PopItemWidth();
        ImGui::TreePop();
    }

    ImGui::Separator();

    ImGui::SetNextItemOpen(collapsedViewOption);
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

    ImGui::SetNextItemOpen(collapsedSA2Option);
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

    bool checkboxes[6] = { 0 };
    checkboxes[defaultSlots[Global::levelID]] = true;

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

            ImGui::BeginDisabled(Global::levelID == 0);
            ImGui::TableSetColumnIndex(0);
            std::string slotText = "Not Set";
            if (Global::levelID != 0 && camLocations.find(Global::levelID) != camLocations.end())
            {
                auto level = camLocations.at(Global::levelID);
                if (level.find(row) != level.end())
                {
                    auto [eye, pitch, yaw] = level.at(row);
                    slotText = std::format("({:.0f}, {:.0f}, {:.0f})", eye.x, eye.y, eye.z);
                }
            }
            ImGui::Text("[%d] %s", row, slotText.c_str());

            ImGui::TableSetColumnIndex(1);
            if (ImGui::Button(std::format("Save##{}", row).c_str()))
            {
                saveCameraLocation(row);
            }
            ImGui::EndDisabled();

            ImGui::BeginDisabled(slotText == "Not Set");
            ImGui::TableSetColumnIndex(2);
            if (ImGui::Button(std::format("Load##{}", row).c_str()))
            {
                loadCameraLocation(row);
            }

            ImGui::TableSetColumnIndex(3);
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 25.0F - ImGui::GetFrameHeight() / 2);

            bool beforeState = checkboxes[row];
            ImGui::Checkbox(std::format("##CheckBox{}", row).c_str(), &checkboxes[row]);
            if (!beforeState && checkboxes[row])
            {
                defaultSlots[Global::levelID] = row;
            }
            else if (beforeState && !checkboxes[row])
            {
                defaultSlots[Global::levelID] = 0;
            }

            ImGui::EndDisabled();
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
        "    Mouse left click + + Shift + WASDQE to move camera at highspeed\n"
        "    * These controls are available while \"Lock Camera\" setting be unchecking only.\n\n"
        "    Shift + F1-F5 to save camera location\n"
        "    F1-F5 to load camera location\n\n"
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
    camLocations[Global::levelID][slotID] = { Global::gameCamera->eye, Global::gameCamera->pitch, Global::gameCamera->yaw };

    return true;
}

bool MenuManager::loadCameraLocation()
{
    return loadCameraLocation(defaultSlots[Global::levelID], true);
}

bool MenuManager::loadCameraLocation(int slotID)
{
    return loadCameraLocation(slotID, false);
}

bool MenuManager::loadCameraLocation(int slotID, bool isDefault)
{
    if (Global::levelID == 0 || camLocations.find(Global::levelID) == camLocations.end())
    {
        return false;
    }

    auto level = camLocations.at(Global::levelID);
    if (level.find(slotID) == level.end())
    {
        return false;
    }

    if (!isDefault && confirmLoad)
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

    auto [eye, pitch, yaw] = level.at(slotID);
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

    saveSettings();
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

void MenuManager::loadSettings()
{
    HRSRC resInfo = FindResource(NULL, MAKEINTRESOURCE(DEFAULT_SETTINGS), "JSON");
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
    std::string settingsJsonStr = ss.str();

    json settings = json::parse(settingsJsonStr);
    try
    {
        std::ifstream f("Settings/MenuSettings.json");

        if (f.is_open())
        {
            json j = json::parse(f);
            settings.merge_patch(j);
        }
        f.close();

        collapsedFileOption = settings.at("/options/file/collapsed"_json_pointer).get_to(collapsedFileOption);

        settings.at("/options/view/collapsed"_json_pointer).get_to(collapsedViewOption);
        settings.at("/options/view/lockCamera"_json_pointer).get_to(lockCamera);
        settings.at("/options/view/displayStage"_json_pointer).get_to(displayStage);
        settings.at("/options/view/displayStageCollision"_json_pointer).get_to(displayStageCollision);
        settings.at("/options/view/displayStageKillplanes"_json_pointer).get_to(displayStageKillplanes);
        settings.at("/options/view/displayStageSky"_json_pointer).get_to(displayStageSky);
        settings.at("/options/view/displayCameraTriggers"_json_pointer).get_to(displayCameraTriggers);
        settings.at("/options/view/displayLoopspeedTriggers"_json_pointer).get_to(displayLoopspeedTriggers);
        settings.at("/options/view/renderWithCulling"_json_pointer).get_to(renderWithCulling);

        settings.at("/options/sa2/collapsed"_json_pointer).get_to(collapsedSA2Option);
        settings.at("/options/sa2/autoLoadObjects"_json_pointer).get_to(autoLoadObjects);
        settings.at("/options/sa2/isFollowRealTime"_json_pointer).get_to(isFollowRealTime);
        settings.at("/options/sa2/gameIsFollowingSA2NoCam"_json_pointer).get_to(gameIsFollowingSA2NoCam);

        settings.at("/options/cameraLocation/confirmSave"_json_pointer).get_to(confirmSave);
        settings.at("/options/cameraLocation/confirmLoad"_json_pointer).get_to(confirmLoad);

        for (auto& level : settings.at("/cameraLocations"_json_pointer))
        {
            int levelID = level.at("/levelID"_json_pointer).get<unsigned int>();
            int defaultSlot = level.value("/default"_json_pointer, 0);

            for (auto& loc : level.at("/locations"_json_pointer))
            {
                int slotID = loc.at("/slotID"_json_pointer).get<unsigned int>();
                if (slotID < 1 || slotID > 5)
                {
                    continue;
                }

                Vector3f eye;
                float pitch;
                float yaw;
                loc.at("/eye/x"_json_pointer).get_to(eye.x);
                loc.at("/eye/y"_json_pointer).get_to(eye.y);
                loc.at("/eye/z"_json_pointer).get_to(eye.z);
                loc.at("/pitch"_json_pointer).get_to(pitch);
                loc.at("/yaw"_json_pointer).get_to(yaw);

                camLocations[levelID][slotID] = { eye, pitch, yaw };
                if (slotID == defaultSlot)
                {
                    defaultSlots[levelID] = slotID;
                }
            }
        }
    }
    catch (json::exception&)
    {

    }
}

void MenuManager::saveSettings()
{
    std::ofstream f("Settings/MenuSettings.json");

    if (!f.is_open())
    {
        f.close();
        return;
    }

    json settings;
    try
    {

        settings["/options/file/collapsed"_json_pointer] = collapsedFileOption;

        settings["/options/view/collapsed"_json_pointer] = collapsedViewOption;
        settings["/options/view/lockCamera"_json_pointer] = lockCamera;
        settings["/options/view/displayStage"_json_pointer] = displayStage;
        settings["/options/view/displayStageCollision"_json_pointer] = displayStageCollision;
        settings["/options/view/displayStageKillplanes"_json_pointer] = displayStageKillplanes;
        settings["/options/view/displayStageSky"_json_pointer] = displayStageSky;
        settings["/options/view/displayCameraTriggers"_json_pointer] = displayCameraTriggers;
        settings["/options/view/displayLoopspeedTriggers"_json_pointer] = displayLoopspeedTriggers;
        settings["/options/view/renderWithCulling"_json_pointer] = renderWithCulling;

        settings["/options/sa2/collapsed"_json_pointer] = collapsedSA2Option;
        settings["/options/sa2/autoLoadObjects"_json_pointer] = autoLoadObjects;
        settings["/options/sa2/isFollowRealTime"_json_pointer] = isFollowRealTime;
        settings["/options/sa2/gameIsFollowingSA2NoCam"_json_pointer] = gameIsFollowingSA2NoCam;

        settings["/options/cameraLocation/confirmSave"_json_pointer] = confirmSave;
        settings["/options/cameraLocation/confirmLoad"_json_pointer] = confirmLoad;

        for (auto& [levelID, level] : camLocations)
        {
            json levelJson;
            levelJson["/levelID"_json_pointer] = levelID;
            levelJson["/default"_json_pointer] = defaultSlots[levelID];

            for (auto& [slotID, loc] : level)
            {
                json locJson;
                auto [eye, pitch, yaw] = level.at(slotID);
                locJson["/slotID"_json_pointer] = slotID;
                locJson["/eye/x"_json_pointer] = eye.x;
                locJson["/eye/y"_json_pointer] = eye.y;
                locJson["/eye/z"_json_pointer] = eye.z;
                locJson["/pitch"_json_pointer] = pitch;
                locJson["/yaw"_json_pointer] = yaw;
                levelJson["/locations"_json_pointer].push_back(locJson);
            }
            settings["/cameraLocations"_json_pointer].push_back(levelJson);
        }
    }
    catch (json::exception&)
    {

    }

    f << settings.dump(4);
    f.close();
}

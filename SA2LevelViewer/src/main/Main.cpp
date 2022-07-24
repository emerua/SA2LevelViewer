#ifdef _WIN32
#define _CRT_SECURE_NO_DEPRECATE
#endif

#include <Windows.h>
#include <commdlg.h>
#include <tchar.h>
#include <tlhelp32.h>

#include <iostream>
#include <fstream>
#include <filesystem>
#include <functional>

#include <string>
#include <cstring>
#include <unordered_map>
#include <unordered_set>
#include <list>

#include <ctime>
#include <random>

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "main.h"
#include "displaymanager.h"
#include "../toolbox/input.h"
#include "../models/rawmodel.h"
#include "../models/modeltexture.h"
#include "../entities/entity.h"
#include "../toolbox/vector.h"
#include "../toolbox/matrix.h"
#include "../entities/camera.h"
#include "../entities/stage.h"
#include "../loading/levelloader.h"
#include "../collision/collisionchecker.h"
#include "../toolbox/split.h"
#include "../toolbox/level.h"
#include "../toolbox/getline.h"
#include "../rendering/masterrenderer.h"
#include "../guis/guimanager.h"
#include "../guis/guirenderer.h"
#include "../guis/guitextureresources.h"
#include "../loading/loader.h"
#include "../entities/cursor3d.h"
#include "../entities/stagecollision.h"
#include "../entities/stagekillplanes.h"
#include "../entities/stagesky.h"
#include "../entities/unknown.h"
#include "../toolbox/maths.h"
#include "../loading/objloader.h"
#include "../entities/dummy.h"
#include "../entities/GlobalObjects/ring.h"
#include "../entities/GlobalObjects/sprb.h"
#include "../entities/GlobalObjects/spra.h"
#include "../entities/GlobalObjects/kasoku.h"
#include "../entities/GlobalObjects/ccube.h"
#include "../entities/GlobalObjects/sphere.h"
#include "../entities/GlobalObjects/emerald.h"
#include "../entities/GlobalObjects/ccyl.h"
#include "../entities/GlobalObjects/bigjump.h"
#include "../entities/GlobalObjects/lightsw.h"
#include "../entities/GlobalObjects/rocket.h"
#include "../entities/GlobalObjects/linklink.h"
#include "../entities/GlobalObjects/stoplockon.h"
#include "../entities/GlobalObjects/bunchin.h"
#include "../entities/GlobalObjects/switch.h"
#include "../entities/GlobalObjects/ironball2.h"
#include "../entities/GlobalObjects/knudai.h"
#include "../entities/GlobalObjects/itembox.h"
#include "../entities/GlobalObjects/itemboxair.h"
#include "../entities/GlobalObjects/itemboxballoon.h"
#include "../entities/GlobalObjects/savepoint.h"
#include "../entities/GlobalObjects/3spring.h"
#include "../entities/GlobalObjects/ekumi.h"
#include "../entities/GlobalObjects/eai.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "MenuManager.h"

std::string Global::version = "1.0.0";

std::unordered_set<Entity*> Global::gameEntities;
std::list<Entity*> Global::gameEntitiesToAdd;
std::list<Entity*> Global::gameEntitiesToDelete;

std::unordered_set<Entity*> Global::gameEntitiesPass2;
std::list<Entity*> Global::gameEntitiesPass2ToAdd;
std::list<Entity*> Global::gameEntitiesPass2ToDelete;

std::unordered_set<Entity*> Global::gameEntitiesPass3;
std::list<Entity*> Global::gameEntitiesPass3ToAdd;
std::list<Entity*> Global::gameEntitiesPass3ToDelete;

std::unordered_set<Entity*> Global::gameTransparentEntities;
std::list<Entity*> Global::gameTransparentEntitiesToAdd;
std::list<Entity*> Global::gameTransparentEntitiesToDelete;


float  dt = 0;
double timeOld = 0;
double timeNew = 0;
bool Global::redrawWindow = true;
SA2Object* Global::selectedSA2Object = nullptr;

std::unordered_set<int> Global::isHoldingKeys;
bool Global::CameraRotateMode = false;

Camera*          Global::gameCamera          = nullptr;
Stage*           Global::gameStage           = nullptr;
StageCollision*  Global::gameStageCollision  = nullptr;
StageKillplanes* Global::gameStageKillplanes = nullptr;
StageSky*        Global::gameStageSky        = nullptr;
Dummy*           Global::gamePlayer          = nullptr;

std::list<TexturedModel*> playerModels;

std::string Global::dirSA2Root = "C:/Program Files (x86)/Steam/steamapps/common/Sonic Adventure 2";
std::string Global::dirProgRoot = "";
std::string Global::dirSet = "";

int Global::countNew = 0;
int Global::countDelete = 0;
int Global::gameState = 0;
int Global::levelID = 0;
int Global::sa2Type = Global::SA2Type::None;
bool Global::shouldLoadNewLevel = false;
bool Global::shouldExportLevel = false;
bool Global::isLoadedLevel = false;

MenuManager* Global::menuManager = nullptr;

int Global::gameMissionNumber = 0;
std::unordered_map<std::string, std::string> Global::levelSetToLVL2;


bool isFollowRealTime = false;

//entry point of the program
int WINAPI WinMain(_In_ HINSTANCE /*hInstance*/, _In_opt_ HINSTANCE /*hPrevInstance*/, _In_ LPSTR /*lpCmdLine*/, _In_ int /*nCmdShow*/)
{
    //remake the console, since it somehow becomes missing once we enter the program here
    #ifdef DEV_MODE
    AllocConsole();
    freopen("CONIN$", "r",stdin);
    freopen("CONOUT$", "w",stdout);
    freopen("CONOUT$", "w",stderr);
    #endif

    char tmp[MAX_PATH] = { 0 };
    char progRoot[MAX_PATH] = { 0 };

    GetModuleFileName(NULL, tmp, MAX_PATH);
    GetLongPathName(tmp, progRoot, sizeof(progRoot));
    std::filesystem::path progRootPath(progRoot);
    Global::dirProgRoot = progRootPath.remove_filename().string();

    std::filesystem::path sa2RootPath(Global::dirSA2Root + "\\resource\\gd_PC\\");
    std::filesystem::current_path(sa2RootPath);

    return Global::main();
}

int Global::main()
{
    MessageBox(NULL, (("Version " + Global::version) +"\nProgram is still a work in progress.").c_str(), "SA2 Level Viewer", MB_OK);

    Global::countNew = 0;
    Global::countDelete = 0;

    srand(0);

    #if !defined(DEV_MODE) && defined(_WIN32)
    //FreeConsole();
    #endif

    DisplayManager::createDisplay();

    Input::init();

    MasterRenderer::init();

    //This camera is never deleted.
    Camera cam;
    Global::gameCamera = &cam;

    LevelLoader::loadLevelData();

    GuiManager::init();
    GuiTextureResources::loadGuiTextures();

    CollisionChecker::initChecker();

    //This stage never gets deleted.
    Stage stage;
    Global::gameStage = &stage;

    //This stage collision never gets deleted.
    StageCollision stageCollision;
    Global::gameStageCollision = &stageCollision;

    //This stage killplanes never gets deleted.
    StageKillplanes stageKillplanes;
    Global::gameStageKillplanes = &stageKillplanes;

    //This sky never gets deleted.
    StageSky stageSky;
    Global::gameStageSky = &stageSky;

    //Load all global object models
    #ifndef OBS_MODE
    loadModel(&playerModels, Global::dirProgRoot + "res/Models/GlobalObjects/Sonic/", "Sonic");
    RING::loadStaticModels();
    Unknown::loadStaticModels();
    SPRB::loadStaticModels();
    SPRA::loadStaticModels();
    KASOKU::loadStaticModels();
    CCUBE::loadStaticModels();
    CCYL::loadStaticModels();
    SPHERE::loadStaticModels();
    EMERALD::loadStaticModels();
    BIGJUMP::loadStaticModels();
    LIGHT_SW::loadStaticModels();
    ROCKET::loadStaticModels();
    LINKLINK::loadStaticModels();
    STOPLOCKON::loadStaticModels();
    BUNCHIN::loadStaticModels();
    SWITCH::loadStaticModels();
    IRONBALL2::loadStaticModels();
    KNUDAI::loadStaticModels();
    ITEMBOX::loadStaticModels();
    ITEMBOXAIR::loadStaticModels();
    ITEMBOXBALLOON::loadStaticModels();
    SAVEPOINT::loadStaticModels();
    THREESPRING::loadStaticModels();
    E_KUMI::loadStaticModels();
    E_AI::loadStaticModels();
    #endif

    //This dummy never gets deleted
    Dummy player(&playerModels);
    Global::gamePlayer = &player;

    #ifdef OBS_MODE
    Global::gamePlayer->visible = false;
    #endif

    Global::menuManager = new MenuManager(DisplayManager::getWindow());

    glfwSetTime(0);

    int frameCount = 0;
    double previousTime = 0;

    Global::gameState = STATE_RUNNING;

    while (Global::gameState != STATE_EXITING && DisplayManager::displayWantsToClose() == 0)
    {
        frameCount++;
        timeNew = glfwGetTime();
        dt = (float)(timeNew - timeOld);
        dt = std::fmaxf(0.001f, dt);

        timeOld = timeNew;

        if (!Global::menuManager->gameIsFollowingSA2 && !Global::menuManager->gameIsFollowingSA2NoCam)
        {
            Input::waitForInputs();
        }
        else
        {
            glfwPollEvents();
        }

        if (Global::shouldLoadNewLevel)
        {
            Global::shouldLoadNewLevel = false;
            #ifndef SAB_LVL
            LevelLoader::promptUserForLevel();
            #else
            LevelLoader::promptUserForLevelSAB();
            #endif
        }

        if (Global::shouldExportLevel)
        {
            Global::shouldExportLevel = false;
            #ifndef SAB_LVL
            LevelLoader::exportLevel();
            #else
            LevelLoader::exportLevelSAB();
            #endif
        }

        if (Global::menuManager->autoLoadObjects)
        {
            LevelLoader::autoLoadLevel();
        }

        GLenum err = glGetError();
        if (err != GL_NO_ERROR)
        {
            std::fprintf(stderr, "########  GL ERROR  ########\n");
            std::fprintf(stderr, "%d\n", err);
        }

        if (Global::menuManager->gameIsFollowingSA2 || Global::menuManager->gameIsFollowingSA2NoCam)
        {
            //Camera prevCam(Global::gameCamera);
            //Vector3f prevPos(&Global::gamePlayer->position);

            Global::updateCamFromSA2();

            //if (prevPos.x == Global::gamePlayer->position.x &&
            //    prevPos.y == Global::gamePlayer->position.y &&
            //    prevPos.z == Global::gamePlayer->position.z &&
            //    Global::gameCamera->equals(&prevCam))
            //{
            //    //nothing has changed, so dont waste cpu+gpu redrawing the same screen
            //    DisplayManager::updateDisplay();
            //    continue;
            //}
        }
        
        if (!Global::menuManager->gameIsFollowingSA2 && Global::CameraRotateMode)
        {
            float moveSpeed = 0.5f;
            if (Global::isHoldingKeys.find(GLFW_KEY_LEFT_SHIFT) != Global::isHoldingKeys.end())
            {
                moveSpeed = 2.0f;
            }

            const std::unordered_map<int, std::function<void(Vector3f&)>> MOVE_FUNCS =
            {
                { GLFW_KEY_W, [moveSpeed](Vector3f& a) { a.x += moveSpeed; } },
                { GLFW_KEY_S, [moveSpeed](Vector3f& a) { a.x -= moveSpeed; } },
                { GLFW_KEY_D, [moveSpeed](Vector3f& a) { a.z += moveSpeed; } },
                { GLFW_KEY_A, [moveSpeed](Vector3f& a) { a.z -= moveSpeed; } },
                { GLFW_KEY_E, [moveSpeed](Vector3f& a) { a.y -= moveSpeed; } },
                { GLFW_KEY_Q, [moveSpeed](Vector3f& a) { a.y += moveSpeed; } }
            };

            Vector3f moveSpeeds;
            for (auto it = Global::isHoldingKeys.cbegin(); it != Global::isHoldingKeys.cend(); it++)
            {
                auto it2 = MOVE_FUNCS.find(*it);
                if (it2 != MOVE_FUNCS.end()) {
                    it2->second(moveSpeeds);
                }
            }

            Vector3f camDolly = Global::gameCamera->calcForward();
            Vector3f camTrackUp = Global::gameCamera->calcUp();
            Vector3f camTrackRight = camTrackUp.cross(&camDolly);

            Vector3f dollyOffset = camDolly.scaleCopy(moveSpeeds.getX());
            Vector3f trackRightOffset = camTrackRight.scaleCopy(moveSpeeds.getZ());
            Vector3f trackUpOffset = camTrackUp.scaleCopy(moveSpeeds.getY());

            Global::gameCamera->eye = Global::gameCamera->eye + dollyOffset + trackRightOffset + trackUpOffset;
        }

        //long double thisTime = std::time(0);
        //std::fprintf(stdout, "time: %f time\n", thisTime);

        //entities managment
        for (auto entityToAdd : gameEntitiesToAdd)
        {
            gameEntities.insert(entityToAdd);
        }
        gameEntitiesToAdd.clear();

        for (auto entityToDelete : gameEntitiesToDelete)
        {
            gameEntities.erase(entityToDelete);
            delete entityToDelete; INCR_DEL("Entity");
        }
        gameEntitiesToDelete.clear();


        //entities pass2 managment
        for (auto entityToAdd : gameEntitiesPass2ToAdd)
        {
            gameEntitiesPass2.insert(entityToAdd);
        }
        gameEntitiesPass2ToAdd.clear();

        for (auto entityToDelete : gameEntitiesPass2ToDelete)
        {
            gameEntitiesPass2.erase(entityToDelete);
            delete entityToDelete; INCR_DEL("Entity");
        }
        gameEntitiesPass2ToDelete.clear();


        //entities pass3 managment
        for (auto entityToAdd : gameEntitiesPass3ToAdd)
        {
            gameEntitiesPass3.insert(entityToAdd);
        }
        gameEntitiesPass3ToAdd.clear();

        for (auto entityToDelete : gameEntitiesPass3ToDelete)
        {
            gameEntitiesPass3.erase(entityToDelete);
            delete entityToDelete; INCR_DEL("Entity");
        }
        gameEntitiesPass3ToDelete.clear();


        //transnaprent entities managment
        for (auto entityToAdd : gameTransparentEntitiesToAdd)
        {
            gameTransparentEntities.insert(entityToAdd);
        }
        gameTransparentEntitiesToAdd.clear();

        for (auto entityToDelete : gameTransparentEntitiesToDelete)
        {
            gameTransparentEntities.erase(entityToDelete);
            delete entityToDelete; INCR_DEL("Entity");
        }
        gameTransparentEntitiesToDelete.clear();

        switch (Global::gameState)
        {
            case STATE_RUNNING:
            {
                //game logic

                for (Entity* e : gameEntities)
                {
                    e->step();
                }
                for (Entity* e : gameEntitiesPass2)
                {
                    e->step();
                }
                for (Entity* e : gameEntitiesPass3)
                {
                    e->step();
                }
                for (Entity* e : gameTransparentEntities)
                {
                    e->step();
                }

                Global::gameStage->step();
                Global::gameStageCollision->step();
                Global::gameStageKillplanes->step();
                Global::gameStageSky->step();

                break;
            }

            default:
                break;
        }

        //prepare entities to render
        for (Entity* e : gameEntities)
        {
            MasterRenderer::processEntity(e);
        }
        for (Entity* e : gameEntitiesPass2)
        {
            MasterRenderer::processEntityPass2(e);
        }
        for (Entity* e : gameEntitiesPass3)
        {
            MasterRenderer::processEntityPass3(e);
        }
        for (Entity* e : gameTransparentEntities)
        {
            MasterRenderer::processTransparentEntity(e);
        }
        
        MasterRenderer::processEntity(Global::gameStage);
        MasterRenderer::processEntity(Global::gameStageCollision);
        MasterRenderer::processEntity(Global::gameStageKillplanes);
        MasterRenderer::processEntity(Global::gameStageSky);
        MasterRenderer::processEntity(Global::gamePlayer);

        glEnable(GL_CLIP_DISTANCE1);
        MasterRenderer::render(&cam);
        glDisable(GL_CLIP_DISTANCE1);


        MasterRenderer::clearEntities();
        MasterRenderer::clearEntitiesPass2();
        MasterRenderer::clearEntitiesPass3();
        MasterRenderer::clearTransparentEntities();

        GuiManager::renderAll();

        //calculate the total number of sa2 objects there are
        int totalSA2Objects = 0;
        for (Entity* e : gameEntities)
        {
            if (e->isSA2Object())
            {
                if (SA2Object* o = dynamic_cast<SA2Object*>(e))
                {
                    totalSA2Objects+=1;
                }
                else
                {
                    std::fprintf(stdout, "Warning: object lied about being an sa2object\n");
                }
            }
        }

        Global::menuManager->InitRender();
        Global::menuManager->CreateViewWindow();
        Global::menuManager->CreateCameraWindow();
        Global::menuManager->CreateHelpWindow();
        Global::menuManager->Render();

        Global::menuManager->gameIsFollowingSA2 = Global::menuManager->isFollowRealTime && !Global::menuManager->gameIsFollowingSA2NoCam;
        Global::menuManager->gameIsFollowingSA2NoCam = Global::menuManager->isFollowRealTime && Global::menuManager->gameIsFollowingSA2NoCam;

        std::string title = "SA2 Level Viewer.  Level ID: "+std::to_string(Global::levelID)+"  Object Count: "+std::to_string(totalSA2Objects);
        glfwSetWindowTitle(DisplayManager::getWindow(), title.c_str());

        DisplayManager::updateDisplay();

        if (timeNew - previousTime >= 1.0)
        {
            //char const* const format = "fps: %f\n";
            //int len = _scprintf(format, frameCount / (timeNew - previousTime)) + 1;
            //std::string buffer(len, '\0');
            //if (buffer.size() != 0)
            //{
            //    sprintf_s(&buffer[0], buffer.size(), format, frameCount / (timeNew - previousTime));
            //}
            //OutputDebugString(buffer.c_str());
            //std::fprintf(stdout, "diff: %d\n", Global::countNew - Global::countDelete);
            //Loader::printInfo();
            //std::fprintf(stdout, "entity counts: %d %d %d\n", gameEntities.size(), gameEntitiesPass2.size(), gameTransparentEntities.size());
            frameCount = 0;
            previousTime = timeNew;
        }

        //std::fprintf(stdout, "dt: %f\n", dt);
        //std::this_thread::sleep_for(std::chrono::milliseconds(8));
    }

    delete Global::menuManager;

    MasterRenderer::cleanUp();
    Loader::cleanUp();
    GuiRenderer::cleanUp();
    DisplayManager::closeDisplay();

    return 0;
}

//The newEntity should be created with the new keyword, as it will be deleted later
void Global::addEntity(Entity* entityToAdd)
{
    gameEntitiesToAdd.push_back(entityToAdd);
}

void Global::deleteEntity(Entity* entityToDelete)
{
    gameEntitiesToDelete.push_back(entityToDelete);
}

void Global::deleteAllEntites()
{
    //Make sure no entities get left behind in transition
    for (Entity* entityToAdd : gameEntitiesToAdd)
    {
        gameEntities.insert(entityToAdd);
    }
    gameEntitiesToAdd.clear();

    for (Entity* entityToDelete : gameEntitiesToDelete)
    {
        gameEntities.erase(entityToDelete);
        delete entityToDelete; INCR_DEL("Entity");
    }
    gameEntitiesToDelete.clear();


    //Delete all the rest
    for (Entity* entityToDelete : gameEntities)
    {
        delete entityToDelete; INCR_DEL("Entity");
    }
    gameEntities.clear();
}

void Global::addEntityPass2(Entity* entityToAdd)
{
    gameEntitiesPass2ToAdd.push_back(entityToAdd);
}

void Global::deleteEntityPass2(Entity* entityToDelete)
{
    gameEntitiesPass2ToDelete.push_back(entityToDelete);
}

void Global::deleteAllEntitesPass2()
{
    //Make sure no entities get left behind in transition
    for (Entity* entityToAdd : gameEntitiesPass2ToAdd)
    {
        gameEntitiesPass2.insert(entityToAdd);
    }
    gameEntitiesPass2ToAdd.clear();

    for (Entity* entityToDelete : gameEntitiesPass2ToDelete)
    {
        gameEntitiesPass2.erase(entityToDelete);
        delete entityToDelete; INCR_DEL("Entity");
    }
    gameEntitiesPass2ToDelete.clear();

    for (Entity* entityToDelete : gameEntitiesPass2)
    {
        delete entityToDelete; INCR_DEL("Entity");
    }
    gameEntitiesPass2.clear();
}

void Global::addEntityPass3(Entity* entityToAdd)
{
    gameEntitiesPass3ToAdd.push_back(entityToAdd);
}

void Global::deleteEntityPass3(Entity* entityToDelete)
{
    gameEntitiesPass3ToDelete.push_back(entityToDelete);
}

void Global::deleteAllEntitesPass3()
{
    //Make sure no entities get left behind in transition
    for (Entity* entityToAdd : gameEntitiesPass3ToAdd)
    {
        gameEntitiesPass3.insert(entityToAdd);
    }
    gameEntitiesPass3ToAdd.clear();

    for (Entity* entityToDelete : gameEntitiesPass3ToDelete)
    {
        gameEntitiesPass3.erase(entityToDelete);
        delete entityToDelete; INCR_DEL("Entity");
    }
    gameEntitiesPass3ToDelete.clear();

    for (Entity* entityToDelete : gameEntitiesPass3)
    {
        delete entityToDelete; INCR_DEL("Entity");
    }
    gameEntitiesPass3.clear();
}

//Transparent entities shouldn't create new transparent entities from within their step function
void Global::addTransparentEntity(Entity* entityToAdd)
{
    gameTransparentEntities.insert(entityToAdd);
}

void Global::deleteTransparentEntity(Entity* entityToDelete)
{
    gameTransparentEntities.erase(entityToDelete);
    delete entityToDelete; INCR_DEL("Entity");
}

void Global::deleteAllTransparentEntites()
{
    for (Entity* entityToDelete : gameTransparentEntities)
    {
        delete entityToDelete; INCR_DEL("Entity");
    }
    gameTransparentEntities.clear();
}

std::unordered_map<std::string, int> heapObjects;

#ifdef DEV_MODE
void Global::debugNew(const char* name)
{
    Global::countNew++;

    if (heapObjects.find(name) == heapObjects.end())
    {
        heapObjects[name] = 1;
    }
    else
    {
        int num = heapObjects[name];
        heapObjects[name] = num+1;
    }
}

void Global::debugDel(const char* name)
{
    Global::countDelete++;

    if (heapObjects.find(name) == heapObjects.end())
    {
        std::fprintf(stdout, "Warning: trying to delete '%s' when there are none.\n", name);
        heapObjects[name] = 0;
    }
    else
    {
        int num = heapObjects[name];
        heapObjects[name] = num-1;
    }
}
#else
void Global::debugNew(const char* )
{
    Global::countNew++;
}

void Global::debugDel(const char* )
{
    Global::countDelete++;
}
#endif


const float ATTACH_DELAY = 2.0f;
DWORD sa2PID = NULL;
HANDLE sa2Handle = NULL;
float timeUntilNextProcessAttach = ATTACH_DELAY;

DWORD getPIDByName(const char* processName)
{
    PROCESSENTRY32 pe32 = {0};
    HANDLE hSnapshot = NULL;

    pe32.dwSize = sizeof(PROCESSENTRY32);
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    bool foundProcess = false;

    if (Process32First(hSnapshot, &pe32))
    {
        do
        {
            if (strcmp(pe32.szExeFile, processName) == 0)
            {
                foundProcess = true;
                break;
            }
        }
        while (Process32Next(hSnapshot, &pe32));
    }

    if (foundProcess && hSnapshot != INVALID_HANDLE_VALUE)
    {
        CloseHandle(hSnapshot);
        return pe32.th32ProcessID;
    }

    return NULL;
}

void Global::attachSA2Process()
{
    sa2PID = NULL;
    sa2Handle = NULL;

    if (timeUntilNextProcessAttach <= 0.0f)
    {
        Global::sa2Type = Global::SA2Type::None;

        sa2PID = getPIDByName("sonic2app.exe");

        if (sa2PID == NULL)
        {
            sa2PID = getPIDByName("Dolphin.exe");

            if (sa2PID == NULL)
            {
                sa2PID = getPIDByName("sonic.exe");

                if (sa2PID == NULL)
                {
                    timeUntilNextProcessAttach = ATTACH_DELAY;
                }
                else
                {
                    Global::sa2Type = Global::SA2Type::Sadx;
                }
            }
            else
            {
                Global::sa2Type = Global::SA2Type::Dolphin;
            }
        }
        else
        {
            Global::sa2Type = Global::SA2Type::Steam;
        }

        if (sa2PID != NULL)
        {
            sa2Handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, false, sa2PID);
            if (sa2Handle == NULL)
            {
                std::fprintf(stdout, "Error: Found the process, but couldn't open and get a handle.\n");
                sa2PID = NULL;
                timeUntilNextProcessAttach = ATTACH_DELAY;
            }
        }
    }
    else
    {
        extern float dt;
        timeUntilNextProcessAttach -= dt;
    }

    return;
}


char Global::getMenuMode()
{
    char menuMode = 0;

    if (sa2Handle == NULL || sa2PID == NULL)
    {
        Global::attachSA2Process();
        return menuMode;
    }

    if (Global::sa2Type == Global::SA2Type::Steam)
    {
        SIZE_T bytesRead = 0;

        if (!ReadProcessMemory(sa2Handle, (LPCVOID)0x1934BE0, (LPVOID)(&menuMode), sizeof(menuMode), &bytesRead) || bytesRead != sizeof(menuMode))
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return menuMode;
        }
    }

    return menuMode;
}

char Global::getCurrentLevel()
{
    char currentLevel = 0;

    if (sa2Handle == NULL || sa2PID == NULL)
    {
        Global::attachSA2Process();
        return currentLevel;
    }

    if (Global::sa2Type == Global::SA2Type::Steam)
    {
        SIZE_T bytesRead = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)0x1934B70, (LPVOID)(&currentLevel), sizeof(currentLevel), &bytesRead) || bytesRead != sizeof(currentLevel))
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return currentLevel;
        }
    }

    return currentLevel;
}

void Global::updateCamFromSA2()
{
    if (sa2Handle == NULL || sa2PID == NULL)
    {
        Global::attachSA2Process();
        return;
    }

    if (Global::sa2Type == Global::SA2Type::Steam)
    {
        SIZE_T bytesRead = 0;
        char buffer[20] = {0};
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)0x1DCFF0C, (LPVOID)buffer, (SIZE_T)20, &bytesRead) || bytesRead != 20)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        float camX;
        char* xptr = (char*)&camX;
        xptr[0] = buffer[0];
        xptr[1] = buffer[1];
        xptr[2] = buffer[2];
        xptr[3] = buffer[3];

        float camY;
        char* yptr = (char*)&camY;
        yptr[0] = buffer[4];
        yptr[1] = buffer[5];
        yptr[2] = buffer[6];
        yptr[3] = buffer[7];

        float camZ;
        char* zptr = (char*)&camZ;
        zptr[0] = buffer[8];
        zptr[1] = buffer[9];
        zptr[2] = buffer[10];
        zptr[3] = buffer[11];

        int pitch;
        memcpy(&pitch, &buffer[12], 4);
    
        int yaw;
        memcpy(&yaw, &buffer[16], 4);

        if (Global::menuManager->gameIsFollowingSA2 && !Global::menuManager->gameIsFollowingSA2NoCam)
        {
            Global::gameCamera->eye.x = camX;
            Global::gameCamera->eye.y = camY;
            Global::gameCamera->eye.z = camZ;
            Global::gameCamera->yaw = -Maths::bamsToDeg(yaw);
            Global::gameCamera->pitch = -Maths::bamsToDeg(pitch);
        }

        //0x019ED3FC = global position copy
        bytesRead = 0;
        unsigned long long masterobjptr = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)0x01DEA6E0, (LPVOID)(&masterobjptr), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        unsigned long long chobj1 = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(masterobjptr+0x34), (LPVOID)(&chobj1), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        float sonicX = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(chobj1+0x14), (LPVOID)(&sonicX), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        float sonicY = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(chobj1+0x18), (LPVOID)(&sonicY), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        float sonicZ = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(chobj1+0x1C), (LPVOID)(&sonicZ), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        int bamsX = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(chobj1+0x08), (LPVOID)(&bamsX), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        int bamsY = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(chobj1+0x0C), (LPVOID)(&bamsY), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        int bamsZ = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(chobj1+0x10), (LPVOID)(&bamsZ), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        Global::gamePlayer->setPosition(sonicX, sonicY, sonicZ);
        Global::gamePlayer->setRotation(bamsX, -bamsY, bamsZ);
        Global::gamePlayer->updateTransformationMatrixYXZ();

        //make the score have a 1 at the end as a first
        // line of defence against cheaters :)
        char score;
        bytesRead = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)0x0174B050, (LPVOID)(&score), (SIZE_T)1, &bytesRead) || bytesRead != 1)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        score = score | 0x1;
        SIZE_T bytesWritten = 0;
        if (!WriteProcessMemory(sa2Handle, (LPVOID)0x0174B050, (LPCVOID)(&score), (SIZE_T)1, &bytesWritten) || bytesWritten != 1)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }
    }
    else if (Global::sa2Type == Global::SA2Type::Dolphin)
    {
        //address of where sa2's memory begins in Dolphin's memory space
        // this doesnt seem to be working anymore
        const unsigned long long DOLPHIN_START_ADDR = 0x7FFF0000ULL;

        //read in camera values
        {
            const unsigned long long CAMADDR = DOLPHIN_START_ADDR + 0x801FF5B8ULL;

            SIZE_T bytesRead = 0;
            char buffer[20] = {0};
            if (!ReadProcessMemory(sa2Handle, (LPCVOID)(CAMADDR), (LPVOID)buffer, (SIZE_T)20, &bytesRead) || bytesRead != 20)
            {
                printf("Error when reading from dolphin\n");
                CloseHandle(sa2Handle);
                sa2Handle = NULL;
                sa2PID = NULL;
                timeUntilNextProcessAttach = ATTACH_DELAY;
                return;
            }

            float camX;
            char* xptr = (char*)&camX;
            xptr[3] = buffer[0];
            xptr[2] = buffer[1];
            xptr[1] = buffer[2];
            xptr[0] = buffer[3];

            float camY;
            char* yptr = (char*)&camY;
            yptr[3] = buffer[4];
            yptr[2] = buffer[5];
            yptr[1] = buffer[6];
            yptr[0] = buffer[7];
        
            float camZ;
            char* zptr = (char*)&camZ;
            zptr[3] = buffer[ 8];
            zptr[2] = buffer[ 9];
            zptr[1] = buffer[10];
            zptr[0] = buffer[11];

            int pitch = 0;
            char* ptr = (char*)&pitch;
            memcpy(ptr+1, &buffer[14], 1);
            memcpy(ptr+0, &buffer[15], 1);
    
            int yaw = 0;
            ptr = (char*)&yaw;
            memcpy(ptr+1, &buffer[18], 1);
            memcpy(ptr+0, &buffer[19], 1);

            if (Global::menuManager->gameIsFollowingSA2 && !Global::menuManager->gameIsFollowingSA2NoCam)
            {
                Global::gameCamera->eye.x = camX;
                Global::gameCamera->eye.y = camY;
                Global::gameCamera->eye.z = camZ;
                Global::gameCamera->yaw = -Maths::bamsToDeg(yaw);
                Global::gameCamera->pitch = -Maths::bamsToDeg(pitch);
            }
        }

        //read in character position and rotation
        {
            const unsigned long long PLAYERADDR = DOLPHIN_START_ADDR + 0x801E7768ULL;

            SIZE_T bytesRead = 0;
            char ptrbuf[4] = {0};
            if (!ReadProcessMemory(sa2Handle, (LPCVOID)(PLAYERADDR), (LPVOID)ptrbuf, (SIZE_T)4, &bytesRead) || bytesRead != 4)
            {
                printf("Error when reading from dolphin\n");
                CloseHandle(sa2Handle);
                sa2Handle = NULL;
                sa2PID = NULL;
                timeUntilNextProcessAttach = ATTACH_DELAY;
                return;
            }

            unsigned long long chobj1 = 0;
            char* ptr = (char*)&chobj1;
            memcpy(ptr+0, &ptrbuf[3], 1);
            memcpy(ptr+1, &ptrbuf[2], 1);
            memcpy(ptr+2, &ptrbuf[1], 1);
            memcpy(ptr+3, &ptrbuf[0], 1);

            chobj1 = DOLPHIN_START_ADDR + chobj1;
            
            bytesRead = 0;
            char buffer[32] = {0};
            if (!ReadProcessMemory(sa2Handle, (LPCVOID)(chobj1), (LPVOID)buffer, (SIZE_T)32, &bytesRead) || bytesRead != 32)
            {
                CloseHandle(sa2Handle);
                sa2Handle = NULL;
                sa2PID = NULL;
                timeUntilNextProcessAttach = ATTACH_DELAY;
                return;
            }

            int bamsX = 0;
            ptr = (char*)&bamsX;
            memcpy(ptr+1, &buffer[10], 1);
            memcpy(ptr+0, &buffer[11], 1);
    
            int bamsY = 0;
            ptr = (char*)&bamsY;
            memcpy(ptr+1, &buffer[14], 1);
            memcpy(ptr+0, &buffer[15], 1);

            int bamsZ = 0;
            ptr = (char*)&bamsZ;
            memcpy(ptr+1, &buffer[18], 1);
            memcpy(ptr+0, &buffer[19], 1);

            float posX;
            ptr = (char*)&posX;
            ptr[3] = buffer[20];
            ptr[2] = buffer[21];
            ptr[1] = buffer[22];
            ptr[0] = buffer[23];

            float posY;
            ptr = (char*)&posY;
            ptr[3] = buffer[24];
            ptr[2] = buffer[25];
            ptr[1] = buffer[26];
            ptr[0] = buffer[27];
        
            float posZ;
            ptr = (char*)&posZ;
            ptr[3] = buffer[28];
            ptr[2] = buffer[29];
            ptr[1] = buffer[30];
            ptr[0] = buffer[31];

            Global::gamePlayer->setPosition(posX, posY, posZ);
            Global::gamePlayer->setRotation(bamsX, -bamsY, bamsZ);
            Global::gamePlayer->updateTransformationMatrixYXZ();
        }
    }
    else if (Global::sa2Type == Global::SA2Type::Sadx)
    {
        //read in camera
        SIZE_T bytesRead = 0;
        unsigned long long camObj1 = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)0x03B2CBB0, (LPVOID)(&camObj1), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        int camRotX = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(camObj1+0x14), (LPVOID)(&camRotX), (SIZE_T)2, &bytesRead) || bytesRead != 2)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        int camRotY = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(camObj1+0x18), (LPVOID)(&camRotY), (SIZE_T)2, &bytesRead) || bytesRead != 2)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        int camRotZ = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(camObj1+0x1C), (LPVOID)(&camRotZ), (SIZE_T)2, &bytesRead) || bytesRead != 2)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        float camPosX = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(camObj1+0x20), (LPVOID)(&camPosX), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        float camPosY = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(camObj1+0x24), (LPVOID)(&camPosY), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        float camPosZ = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(camObj1+0x28), (LPVOID)(&camPosZ), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        //read in player
        bytesRead = 0;
        unsigned long long playerObj1 = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)0x03B42E10, (LPVOID)(&playerObj1), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        int playerRotX = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(playerObj1+0x14), (LPVOID)(&playerRotX), (SIZE_T)2, &bytesRead) || bytesRead != 2)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        int playerRotY = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(playerObj1+0x18), (LPVOID)(&playerRotY), (SIZE_T)2, &bytesRead) || bytesRead != 2)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        int playerRotZ = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(playerObj1+0x1C), (LPVOID)(&playerRotZ), (SIZE_T)2, &bytesRead) || bytesRead != 2)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        float playerPosX = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(playerObj1+0x20), (LPVOID)(&playerPosX), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        float playerPosY = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(playerObj1+0x24), (LPVOID)(&playerPosY), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        bytesRead = 0;
        float playerPosZ = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)(playerObj1+0x28), (LPVOID)(&playerPosZ), (SIZE_T)4, &bytesRead) || bytesRead != 4)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }

        Global::gamePlayer->setPosition(playerPosX, playerPosY, playerPosZ);
        Global::gamePlayer->setRotation(playerRotX, -playerRotY, playerRotZ);
        Global::gamePlayer->updateTransformationMatrixYXZ();

        if (Global::menuManager->gameIsFollowingSA2 && !Global::menuManager->gameIsFollowingSA2NoCam)
        {
            Global::gameCamera->eye.x = camPosX;
            Global::gameCamera->eye.y = camPosY;
            Global::gameCamera->eye.z = camPosZ;
            Global::gameCamera->yaw = -Maths::bamsToDeg(camRotY);
            Global::gameCamera->pitch = -Maths::bamsToDeg(camRotX);
        }

        //make the score have a 1 at the end as a first
        // line of defence against cheaters :)
        char score;
        bytesRead = 0;
        if (!ReadProcessMemory(sa2Handle, (LPCVOID)0x0174B050, (LPVOID)(&score), (SIZE_T)1, &bytesRead) || bytesRead != 1)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }
        
        score = score | 0x1;
        SIZE_T bytesWritten = 0;
        if (!WriteProcessMemory(sa2Handle, (LPVOID)0x0174B050, (LPCVOID)(&score), (SIZE_T)1, &bytesWritten) || bytesWritten != 1)
        {
            CloseHandle(sa2Handle);
            sa2Handle = NULL;
            sa2PID = NULL;
            timeUntilNextProcessAttach = ATTACH_DELAY;
            return;
        }
    }
}

void Global::teleportSA2PlayerToCursor3D()
{
    //DWORD pid = getPIDByName("sonic2app.exe");

    //if (pid == NULL)
    //{
    //    return;
    //}

    //HANDLE handle = OpenProcess(PROCESS_VM_READ | PROCESS_VM_OPERATION | PROCESS_VM_WRITE, false, pid);
    //if (handle == NULL)
    //{
    //    return;
    //}

    //SIZE_T bytesRead = 0;
    //unsigned long long masterobjptr = 0;
    //if (!ReadProcessMemory(handle, (LPCVOID)0x01DEA6E0, (LPVOID)(&masterobjptr), (SIZE_T)4, &bytesRead) || bytesRead != 4)
    //{
    //    CloseHandle(handle);
    //    return;
    //}

    //if (masterobjptr <= 0x400000)
    //{
    //    CloseHandle(handle);
    //    return;
    //}

    //bytesRead = 0;
    //unsigned long long chobj1ptr = 0;
    //if (!ReadProcessMemory(handle, (LPCVOID)(masterobjptr+0x34), (LPVOID)(&chobj1ptr), (SIZE_T)4, &bytesRead) || bytesRead != 4)
    //{
    //    CloseHandle(handle);
    //    return;
    //}

    //if (chobj1ptr <= 0x400000)
    //{
    //    CloseHandle(handle);
    //    return;
    //}

    //SIZE_T bytesWritten = 0;
    //float sonicX = Global::gameCursor3D->position.x;
    //if (!WriteProcessMemory(handle, (LPVOID)(chobj1ptr+0x14), (LPCVOID)(&sonicX), (SIZE_T)4, &bytesWritten) || bytesWritten != 4)
    //{
    //    CloseHandle(handle);
    //    return;
    //}

    //bytesWritten = 0;
    //float sonicY = Global::gameCursor3D->position.y+5;
    //if (!WriteProcessMemory(handle, (LPVOID)(chobj1ptr+0x18), (LPCVOID)(&sonicY), (SIZE_T)4, &bytesWritten) || bytesWritten != 4)
    //{
    //    CloseHandle(handle);
    //    return;
    //}

    //bytesWritten = 0;
    //float sonicZ = Global::gameCursor3D->position.z;
    //if (!WriteProcessMemory(handle, (LPVOID)(chobj1ptr+0x1C), (LPCVOID)(&sonicZ), (SIZE_T)4, &bytesWritten) || bytesWritten != 4)
    //{
    //    CloseHandle(handle);
    //    return;
    //}

    //CloseHandle(handle);
}

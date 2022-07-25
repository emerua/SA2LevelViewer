#include <Windows.h>
#include <commdlg.h>
#include <tchar.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <SOIL/SOIL.h>

#include <iostream>
#include <cmath>
#include <fstream>
#include <string>
#include <cstring>

#include "displaymanager.h"
#include "../toolbox/input.h"
#include "../main/main.h"
#include "../toolbox/split.h"
#include "../toolbox/getline.h"
#include "../rendering/masterrenderer.h"
#include "../entities/camera.h"
#include "../toolbox/maths.h"
#include "../toolbox/vector.h"
#include "../collision/collisionchecker.h"
#include "../entities/cursor3d.h"
#include "../entities/stagecollision.h"
#include "../entities/stagekillplanes.h"
#include "../entities/stagesky.h"
#include "../entities/stage.h"
#include "../loading/levelloader.h"

#include "../entities/GlobalObjects/ring.h"

// default settings
unsigned int DisplayManager::SCR_WIDTH = 1280;
unsigned int DisplayManager::SCR_HEIGHT = 720;
unsigned int DisplayManager::AA_SAMPLES = 0;
GLFWwindow* DisplayManager::glfwWindow = nullptr;

int DisplayManager::createDisplay()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    //glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Request an OpenGL 4.0 core context.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    loadDisplaySettings();
    loadGraphicsSettings();

    GLFWmonitor* monitor = nullptr;

    glfwWindowHint(GLFW_SAMPLES, AA_SAMPLES);
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    unsigned int screenWidth  = SCR_WIDTH;
    unsigned int screenHeight = SCR_HEIGHT;


    //int count;
    //const GLFWvidmode* modes = glfwGetVideoModes(monitor, &count);

    //for (int i = 0; i < count; i++)
    //{
        //std::fprintf(stdout, "%s\n", modes[i].);
    //}

    // glfw window creation
    // --------------------
    glfwWindow = glfwCreateWindow(screenWidth, screenHeight, "SA2 Level Editor", monitor, nullptr);
    if (glfwWindow == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(glfwWindow);
    glfwSetFramebufferSizeCallback(glfwWindow, DisplayManager::framebuffer_size_callback);
    glfwSetWindowCloseCallback(glfwWindow, DisplayManager::window_close_callback);
    glfwSetCursorPosCallback(glfwWindow, DisplayManager::callbackCursorPosition);
    glfwSetScrollCallback(glfwWindow, DisplayManager::callbackMouseScroll);
    glfwSetMouseButtonCallback(glfwWindow, DisplayManager::callbackMouseClick);
    glfwSetKeyCallback(glfwWindow, DisplayManager::callbackKeyboard);

    GLFWimage icons[3];
    std::string icon16 = Global::dirProgRoot + "res/Images/Icon16.png";
    std::string icon32 = Global::dirProgRoot + "res/Images/Icon32.png";
    std::string icon64 = Global::dirProgRoot + "res/Images/Icon64.png";
    icons[0].pixels = SOIL_load_image(icon16.c_str(), &icons[0].width, &icons[0].height, 0, SOIL_LOAD_RGBA);
    icons[1].pixels = SOIL_load_image(icon32.c_str(), &icons[1].width, &icons[1].height, 0, SOIL_LOAD_RGBA);
    icons[2].pixels = SOIL_load_image(icon64.c_str(), &icons[2].width, &icons[2].height, 0, SOIL_LOAD_RGBA);

    if (icons[0].pixels != nullptr &&
        icons[1].pixels != nullptr &&
        icons[2].pixels != nullptr)
    {
        glfwSetWindowIcon(glfwWindow, 3, icons);
        SOIL_free_image_data(icons[0].pixels);
        SOIL_free_image_data(icons[1].pixels);
        SOIL_free_image_data(icons[2].pixels);
    }

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    std::fprintf(stdout, "Renderer:       %s\n", glGetString(GL_RENDERER));
    std::fprintf(stdout, "Vendor:         %s\n", glGetString(GL_VENDOR));
    std::fprintf(stdout, "OpenGL version: %s\n", glGetString(GL_VERSION));
    std::fprintf(stdout, "GLSL version:   %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));

    glfwSwapInterval(1); //1 = vsync. 0 = off. 2 = half monitor refresh rate
    glEnable(GL_MULTISAMPLE);

    //Center the window
    const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    int monitorWidth = mode->width;
    int monitorHeight = mode->height;

    if ((int)SCR_WIDTH  <= monitorWidth && 
        (int)SCR_HEIGHT <= monitorHeight)
    {
        int xpos = monitorWidth/2  - ((int)SCR_WIDTH)/2;
        int ypos = monitorHeight/2 - ((int)SCR_HEIGHT)/2;

        glfwSetWindowPos(glfwWindow, xpos, ypos);
    }

    //glfwGetWindowAttrib(window, GLFW_SAMPLES);
    //std::fprintf(stdout, "samples:   %d\n", glfwGetWindowAttrib(window, GLFW_SAMPLES));

    //float aniso = 0.0f;
    //glGetFloatv(GL_MAX_TEXTURE_LOD_BIAS, &aniso);
    //std::fprintf(stdout, "max lod bias:   %f\n", aniso);

    //To check what extensions are avalible 
    //int ext_cnt;
    //glGetIntegerv(GL_NUM_EXTENSIONS, &ext_cnt);
    //for (int i = 0; i < ext_cnt; i++)
    {
        //std::fprintf(stdout, "extensions:   %s\n", glGetStringi(GL_EXTENSIONS, i));
    }

    //Master_makeProjectionMatrix();

    glfwSetWindowSizeLimits(glfwWindow, ((int)SCR_WIDTH) / 2, ((int)SCR_HEIGHT) / 2, GLFW_DONT_CARE, GLFW_DONT_CARE);

    return 0;
}

void DisplayManager::updateDisplay()
{
    glfwSwapBuffers(glfwWindow);
}

void DisplayManager::closeDisplay()
{
    glfwDestroyWindow(glfwWindow);
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
}

int DisplayManager::displayWantsToClose()
{
    return glfwWindowShouldClose(glfwWindow);
}

GLFWwindow* DisplayManager::getWindow()
{
    return glfwWindow;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void DisplayManager::framebuffer_size_callback(GLFWwindow* /*windowHandle*/, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    MasterRenderer::makeProjectionMatrix();
    Global::redrawWindow = true;
}

double DisplayManager::prevXPos = 0;
double DisplayManager::prevYPos = 0;
void DisplayManager::callbackCursorPosition(GLFWwindow* /*window*/, double xpos, double ypos)
{
    float xDiff = (float)(xpos - prevXPos);
    float yDiff = (float)(ypos - prevYPos);
    DisplayManager::prevXPos = xpos;
    DisplayManager::prevYPos = ypos;

    if (!Global::CameraRotateMode) {
        return;
    }

    const float ROTATE_SPEED = 0.2f;
    Global::gameCamera->yaw += xDiff * ROTATE_SPEED;

    float pitchBefore = Global::gameCamera->pitch;
    float pitchAfter = pitchBefore + yDiff * ROTATE_SPEED;

    pitchAfter = fmaxf(pitchAfter, -89.99f);
    pitchAfter = fminf(pitchAfter, 89.99f);

    Global::gameCamera->pitch = pitchAfter;

    Global::redrawWindow = true;
}

void DisplayManager::callbackMouseScroll(GLFWwindow* /*window*/, double /*xoffset*/, double yoffset)
{
    if (Global::menuManager->lockCamera || Global::menuManager->openLicenseWindow)
    {
        return;
    }

    //move the camera forward or back at constant rate
    const float MOVE_SPEED = 40.0f;
    Vector3f camDir = Global::gameCamera->calcForward();
    camDir.normalize();
    Vector3f moveOffset = camDir.scaleCopy(((float)yoffset)*MOVE_SPEED);
    Global::gameCamera->eye = Global::gameCamera->eye + moveOffset;
    Global::redrawWindow = true;
}

void DisplayManager::callbackMouseClick(GLFWwindow* window, int button, int action, int /*mods*/)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS && !Global::menuManager->lockCamera && !Global::menuManager->isMouseCaptured())
        {
            Global::CameraRotateMode = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else if(action == GLFW_RELEASE && Global::CameraRotateMode)
        {
            Global::CameraRotateMode = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
}

void DisplayManager::callbackKeyboard(GLFWwindow* /*window*/, int key, int /*scancode*/, int action, int mods)
{
    switch (key)
    {
        case GLFW_KEY_L:
            if (action == GLFW_PRESS && !Global::menuManager->autoLoadObjects)
            {
                Global::shouldLoadNewLevel = true;
            }
            break;

        case GLFW_KEY_W:
        case GLFW_KEY_S:
        case GLFW_KEY_D:
        case GLFW_KEY_A:
        case GLFW_KEY_E:
        case GLFW_KEY_Q:
        case GLFW_KEY_LEFT_SHIFT:
            if (action == GLFW_PRESS)
            {
                Global::isHoldingKeys.insert(key);
            }
            else if(action == GLFW_RELEASE)
            {
                Global::isHoldingKeys.erase(key);
            }

            break;

        case GLFW_KEY_F1:
        case GLFW_KEY_F2:
        case GLFW_KEY_F3:
        case GLFW_KEY_F4:
        case GLFW_KEY_F5:
            if (action != GLFW_PRESS)
            {
                break;
            }

            {
                int locNo = key - GLFW_KEY_F1 + 1;
                if (mods && GLFW_MOD_SHIFT)
                {
                    Global::menuManager->saveCameraLocation(locNo);
                }
                else
                {
                    Global::menuManager->loadCameraLocation(locNo);
                }
            }
            break;

        case GLFW_KEY_KP_7:
            if (action == GLFW_PRESS)
            {
                Global::gameCamera->yaw = 0.0f;
                Global::gameCamera->pitch = 89.99f;
                Global::redrawWindow = true;
            }
            break;

        case GLFW_KEY_KP_4:
            if (action == GLFW_PRESS)
            {
                Global::gameCamera->yaw -= 22.5f;
                Global::redrawWindow = true;
            }
            break;

        case GLFW_KEY_KP_6:
            if (action == GLFW_PRESS)
            {
                Global::gameCamera->yaw += 22.5f;
                Global::redrawWindow = true;
            }
            break;

        default:
            break;
    }
}

void DisplayManager::window_close_callback(GLFWwindow* /*windowHandle*/)
{
    Global::gameState = STATE_EXITING;
}

void DisplayManager::loadDisplaySettings()
{
    std::ifstream file("Settings/DisplaySettings.ini");
    if (!file.is_open())
    {
        std::fprintf(stdout, "Error: Cannot load file 'Settings/DisplaySettings.ini'\n");
        file.close();
    }
    else
    {
        std::string line;

        while (!file.eof())
        {
            getlineSafe(file, line);

            char lineBuf[512];
            memcpy(lineBuf, line.c_str(), line.size()+1);

            int splitLength = 0;
            char** lineSplit = split(lineBuf, ' ', &splitLength);

            if (splitLength == 2)
            {
                if (strcmp(lineSplit[0], "Width") == 0)
                {
                    SCR_WIDTH = std::stoi(lineSplit[1], nullptr, 10);
                }
                else if (strcmp(lineSplit[0], "Height") == 0)
                {
                    SCR_HEIGHT = std::stoi(lineSplit[1], nullptr, 10);
                }
            }
            free(lineSplit);
        }
        file.close();
    }
}

void DisplayManager::loadGraphicsSettings()
{
    std::ifstream file("Settings/GraphicsSettings.ini");
    if (!file.is_open())
    {
        std::fprintf(stdout, "Error: Cannot load file 'Settings/GraphicsSettings.ini'\n");
        file.close();
    }
    else
    {
        std::string line;

        while (!file.eof())
        {
            getlineSafe(file, line);

            char lineBuf[512];
            memcpy(lineBuf, line.c_str(), line.size()+1);

            int splitLength = 0;
            char** lineSplit = split(lineBuf, ' ', &splitLength);

            if (splitLength == 2)
            {
                if (strcmp(lineSplit[0], "FOV") == 0)
                {
                    MasterRenderer::DEFAULT_VFOV = std::stof(lineSplit[1], nullptr);
                }
                else if (strcmp(lineSplit[0], "Anti-Aliasing_Samples") == 0)
                {
                    AA_SAMPLES = std::stoi(lineSplit[1], nullptr, 10);
                }
            }
            free(lineSplit);
        }
        file.close();
    }
}

Vector2f DisplayManager::getResolution()
{
    return Vector2f((float)SCR_WIDTH, (float)SCR_HEIGHT);
}

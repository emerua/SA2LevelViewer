#pragma once
#include "../imgui/imgui.h"
#include <GLFW/glfw3.h>
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include <string>

class MenuManager
{
public:

	MenuManager(GLFWwindow* window);

	void InitRender();

	bool CreateViewWindow();

	void CreateHelpWindow(std::string);

	void Render();

	~MenuManager();

	bool gameIsFollowingSA2 = false;
	bool gameIsFollowingSA2NoCam = false;
	bool autoLoadObjects = false;
	bool displayCameraTriggers = false;
	bool displayLoopspeedTriggers = false;
	bool displayStage = true;
	bool displayStageCollision = false;
	bool displayStageKillplanes = false;
	bool displayStageSky = false;
	bool renderWithCulling = false;
	bool isFollowRealTime = false;
};
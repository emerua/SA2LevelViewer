#pragma once
#include "../imgui/imgui.h"
#include <GLFW/glfw3.h>
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../toolbox/vector.h"
#include <unordered_map>
#include <tuple>

class MenuManager
{
public:

	MenuManager(GLFWwindow* window);

	void InitRender();

	void CreateViewWindow();

	void CreateCameraWindow();

	void CreateHelpWindow();

	void CreateLicenseWindow();

	void Render();

	bool isMouseCaptured();

	bool saveCameraLocation(int);

	bool loadCameraLocation(int);

	~MenuManager();

	ImGuiContext* context;
	ImGuiIO& io;

	bool gameIsFollowingSA2 = false;
	bool gameIsFollowingSA2NoCam = false;
	bool autoLoadObjects = false;
	bool lockCamera = true;
	bool displayCameraTriggers = false;
	bool displayLoopspeedTriggers = false;
	bool displayStage = true;
	bool displayStageCollision = false;
	bool displayStageKillplanes = false;
	bool displayStageSky = false;
	bool renderWithCulling = false;
	bool isFollowRealTime = false;

	bool openLicenseWindow = false;

	bool confirmSave = true;
	bool confirmLoad = true;

	std::unordered_map<int, std::tuple<Vector3f, float, float>> camLocations;
private:
	ImVec2 adjustWindow(const char* const);
};
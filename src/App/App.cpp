#include <iostream>
#include <functional>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include "App.h"

namespace CG
{
	App::App()
	{
		mainWindow = nullptr;

		controlWindow          = nullptr;
		stickerHierarchyWindow = nullptr;
		stickerInspectorWindow = nullptr;
		showControlWindow      = true;

		mainScene = nullptr;
	}

	App::~App()
	{}

	auto App::Initialize() -> bool
	{
		glfwSetErrorCallback([](int error, const char* description)
			{ fprintf(stderr, "GLFW Error %d: %s\n", error, description); });

		if (!glfwInit())
			return false;

		const char* glsl_version = "#version 460";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

		mainWindow = glfwCreateWindow(1280, 720, "cg-gui", nullptr, nullptr);
		if (mainWindow == nullptr)
			return false;
		glfwMakeContextCurrent(mainWindow);
		glfwSwapInterval(1);

		glewExperimental = GL_TRUE;
		GLenum glew_err = glewInit();
		if (glew_err != GLEW_OK)
		{
			throw std::runtime_error(std::string("Error initializing GLEW: ")
				+ (const char*)glewGetErrorString(glew_err));
		}

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

		ImGui::StyleColorsDark();

		ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
		ImGui_ImplOpenGL3_Init(glsl_version);

		glfwSetWindowUserPointer(mainWindow, this);
		glfwSetFramebufferSizeCallback(mainWindow,
			[](GLFWwindow* window, int w, int h)
			{
				auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
				app->GetMainScene()->OnResize(w, h);
			});

		// Build scene and UI
		mainScene = new MainScene();
		mainScene->Initialize(1280, 720);

		controlWindow = new ControlWindow();
		controlWindow->Initialize();
		controlWindow->SetTargetScene(mainScene);

		stickerHierarchyWindow = new StickerHierarchyWindow();
		stickerHierarchyWindow->SetTargetScene(mainScene);

		stickerInspectorWindow = new StickerInspectorWindow();
		stickerInspectorWindow->SetTargetScene(mainScene);

		return true;
	}

	void App::Loop()
	{
		while (!glfwWindowShouldClose(mainWindow))
		{
			glfwPollEvents();

			timeNow   = glfwGetTime();
			timeDelta = timeNow - timeLast;
			timeLast  = timeNow;
			Update(timeDelta);

			ImGuiIO& io = ImGui::GetIO();

			// ── Camera control (right-mouse drag) ────────────────────
			if (io.MouseDown[ImGuiMouseButton_Right])
			{
				float dx =  io.MouseDelta.x;
				float dy = -io.MouseDelta.y;
				if (dx != 0.0f || dy != 0.0f)
					mainScene->camera->ProcessMouseMovement(dx, dy);

				std::array<bool, 6> pressedKey = {
					ImGui::IsKeyDown(ImGuiKey_W),
					ImGui::IsKeyDown(ImGuiKey_S),
					ImGui::IsKeyDown(ImGuiKey_A),
					ImGui::IsKeyDown(ImGuiKey_D),
					ImGui::IsKeyDown(ImGuiKey_Q),
					ImGui::IsKeyDown(ImGuiKey_E)
				};
				mainScene->camera->ProcessKeyboard(pressedKey, 0.005);
			}

			// ── Left-click: place or reposition sticker ──────────────
			if (io.MouseClicked[ImGuiMouseButton_Left] && !io.WantCaptureMouse)
			{
				ImVec2 mousePosAbs = ImGui::GetMousePos();
				ImVec2 windowPos   = ImGui::GetMainViewport()->Pos;
				glm::vec2 mousePosRel = glm::vec2(
					mousePosAbs.x - windowPos.x,
					mousePosAbs.y - windowPos.y);

				int display_w, display_h;
				glfwGetFramebufferSize(mainWindow, &display_w, &display_h);
				if (display_w > 0 && display_h > 0)
					mainScene->RayCastTest(mousePosRel, display_w, display_h);

				if (mainScene->hasHitPoint)
				{
					if (mainScene->repositioningMode && mainScene->selectedStickerIndex >= 0)
					{
						// Reposition the selected sticker
						mainScene->stickers[mainScene->selectedStickerIndex].center =
							mainScene->lastHitWorldPos;
						mainScene->repositioningMode = false;
					}
					else
					{
						// Place a new sticker from the current ControlWindow template
						controlWindow->PlaceNewStickerAt(mainScene->lastHitWorldPos);
					}
				}
			}

			// ── ImGui frame ──────────────────────────────────────────
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			if (showControlWindow)
			{
				controlWindow->Display();
				stickerHierarchyWindow->Display();
				stickerInspectorWindow->Display();
			}

			ImGui::Render();

			Render();

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(mainWindow);
		}
	}

	void App::Terminate()
	{
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(mainWindow);
		glfwTerminate();
	}

	void App::Update(double dt)
	{
		mainScene->Update(dt);
	}

	void App::Render()
	{
		int display_w, display_h;
		glfwGetFramebufferSize(mainWindow, &display_w, &display_h);
		if (display_w <= 0 || display_h <= 0) return;
		glViewport(0, 0, display_w, display_h);

		mainScene->Render(display_w, display_h);
	}
}

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

		controlWindow = nullptr;
		showControlWindow = true;

		mainScene = nullptr;
	}

	App::~App()
	{

	}

	auto App::Initialize() -> bool
	{
		// Set error callback
		glfwSetErrorCallback([](int error, const char* description)
			{ fprintf(stderr, "GLFW Error %d: %s\n", error, description); });

		// Initialize GLFW
		if (!glfwInit())
			return false;

		// GL 4.6 + GLSL 460
		const char* glsl_version = "#version 460";
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

		// Create window with graphics context
		mainWindow = glfwCreateWindow(1280, 720, "cg-gui", nullptr, nullptr);
		if (mainWindow == nullptr)
			return false;
		glfwMakeContextCurrent(mainWindow);
		glfwSwapInterval(1); // Enable vsync

		// Initialize GLEW
		glewExperimental = GL_TRUE;
		GLenum glew_err = glewInit();
		if (glew_err != GLEW_OK)
		{
			throw std::runtime_error(std::string("Error initializing GLEW, error: ") + (const char*)glewGetErrorString(glew_err));
			return false;
		}

		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;   // Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();

		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(mainWindow, true);
		ImGui_ImplOpenGL3_Init(glsl_version);

		glfwSetWindowUserPointer(mainWindow, this);
		glfwSetFramebufferSizeCallback(
			mainWindow,
			[](GLFWwindow* window, int w, int h)
			{
				auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
				auto mainScene = app->GetMainScene();
				mainScene->OnResize(w, h);
			}
		);

		controlWindow = new ControlWindow();
		controlWindow->Initialize();

		mainScene = new MainScene();
		mainScene->Initialize(1280, 720);

		controlWindow->SetTargetScene(mainScene);

		// Initialization done
		return true;
	}

	void App::Loop()
	{
		while (!glfwWindowShouldClose(mainWindow))
		{
			// Poll and handle events (inputs, window resize, etc.)
			// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
			// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
			// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
			// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
			glfwPollEvents();

			timeNow = glfwGetTime();
			timeDelta = timeNow - timeLast;
			timeLast = timeNow;
			Update(timeDelta);

			//Input handler
			ImGuiIO& io = ImGui::GetIO();
			if (io.MouseDown[ImGuiMouseButton_Right])
			{
				float dx = io.MouseDelta.x;
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
			if (io.MouseClicked[ImGuiMouseButton_Left])
			{
				ImVec2 mousePosAbs = ImGui::GetMousePos();        // Absolute screen position
				ImVec2 windowPos = ImGui::GetMainViewport()->Pos;        // Window's top-left corner
				glm::vec2 mousePosRel = glm::vec2(mousePosAbs.x - windowPos.x, mousePosAbs.y - windowPos.y);


				int display_w, display_h;
				glfwGetFramebufferSize(mainWindow, &display_w, &display_h);
				mainScene->RayCastTest(mousePosRel, display_w, display_h);
			}

			// Start the Dear ImGui frame
			ImGui_ImplOpenGL3_NewFrame();
			ImGui_ImplGlfw_NewFrame();
			ImGui::NewFrame();

			if (showControlWindow)
			{
				controlWindow->Display();
			}

			// GUI Rendering
			ImGui::Render();

			// Render 3D scene
			Render();

			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			// Update and Render additional Platform Windows
			// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
			//  For this specific demo app we could also call glfwMakeContextCurrent(window) directly)
			//ImGuiIO& io = ImGui::GetIO();
			(void)io;
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}

			glfwSwapBuffers(mainWindow);
		}
	}

	void App::Terminate()
	{
		// Cleanup
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
		glViewport(0, 0, display_w, display_h);

		mainScene->Render(display_w, display_h);
	}
}
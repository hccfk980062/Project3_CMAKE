#pragma once

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\matrix_inverse.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Window/ControlWindow.h"
#include "Scene/MainScene.h"

namespace CG
{
	class App
	{
    public:
        App();
        ~App();

        auto Initialize() -> bool;
        void Loop();
        void Terminate();

    private:
        void Update(double dt);
        void Render();

    private:
        GLFWwindow* mainWindow;

        ControlWindow* controlWindow;
        bool showControlWindow;

        MainScene* mainScene;

        double timeNow = 0;
        double timeLast = 0;
        double timeDelta = 0;

    public:
        MainScene* GetMainScene() const
        {
            return mainScene;
        }
	};
}


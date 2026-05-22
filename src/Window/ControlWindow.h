#pragma once

#include <Scene/MainScene.h>

namespace CG
{
	class ControlWindow
	{
	public:
		ControlWindow();

		auto Initialize() -> bool;
		void Display();

	private:
		bool showDemoWindow;

	private:
		MainScene* targetScene;

	public:
		void SetTargetScene(MainScene* scene)
		{
			targetScene = scene;
		}
	};
}

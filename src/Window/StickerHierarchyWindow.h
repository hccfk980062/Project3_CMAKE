#pragma once

#include <Scene/MainScene.h>
#include <imgui.h>

namespace CG
{
	class StickerHierarchyWindow
	{
	public:
		StickerHierarchyWindow() : targetScene(nullptr) {}
		void SetTargetScene(MainScene* scene) { targetScene = scene; }
		void Display();

	private:
		MainScene* targetScene;
	};
}

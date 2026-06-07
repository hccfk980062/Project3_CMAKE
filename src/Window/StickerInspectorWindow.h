#pragma once

#include <Scene/MainScene.h>
#include <imgui.h>

namespace CG
{
	class StickerInspectorWindow
	{
	public:
		StickerInspectorWindow() : targetScene(nullptr) {}
		void SetTargetScene(MainScene* scene) { targetScene = scene; }
		void Display();

	private:
		MainScene* targetScene;
	};
}

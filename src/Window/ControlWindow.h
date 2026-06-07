#pragma once

#include <Scene/MainScene.h>
#include <vector>
#include <string>
#include <glm/vec2.hpp>
#include <imgui.h>

namespace CG
{
	struct TextureData
	{
		std::string name;
		std::string relativePath;
		ImTextureID id;
		float mix = 0.35f;
		glm::vec2 scale = glm::vec2(1.0f);
		float rotationDegrees = 0.0f;
		glm::vec2 offset = glm::vec2(0.0f);
	};

	class ControlWindow
	{
	public:
		ControlWindow();
		auto Initialize() -> bool;
		void Display();

	private:
		void SaveAssetsToJson();
		void LoadAssetsFromJson(); // <-- 新增：初次啟動時載入 JSON
		void ConvertToRelativePath(const std::string& absolutePath, TextureData& tex);
		auto LoadTextureFromFile(const std::string& filepath) -> ImTextureID;
		auto OpenFileDialog() -> std::string;

	private:
		bool showDemoWindow;
		char texturePath[260] = "";

		std::vector<TextureData> textures;
		TextureData* current_texture = nullptr;

		MainScene* targetScene;

	public:
		void SetTargetScene(MainScene* scene) { targetScene = scene; }
	};
}
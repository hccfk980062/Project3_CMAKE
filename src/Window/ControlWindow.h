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
		float mix        = 0.35f;
		float worldSize  = 0.3f;
		glm::vec2 scale  = glm::vec2(1.0f);   // UV tiling
		float rotationDegrees = 0.0f;
		glm::vec2 offset = glm::vec2(0.0f);
		int projAxis     = 0;
	};

	class ControlWindow
	{
	public:
		ControlWindow();
		auto Initialize() -> bool;
		void Display();
		void SetTargetScene(MainScene* scene);

		// Creates a new StickerState from current template and adds it to the scene
		void PlaceNewStickerAt(const glm::vec3& center);

	private:
		void SaveAssetsToJson();
		void LoadAssetsFromJson();
		void ConvertToRelativePath(const std::string& absolutePath, TextureData& tex);
		auto LoadTextureFromFile(const std::string& filepath) -> ImTextureID;
		auto OpenFileDialog() -> std::string;

	private:
		bool showDemoWindow;
		char texturePath[260] = "";

		std::vector<TextureData> textures;
		TextureData* current_texture = nullptr;

		MainScene* targetScene;
	};
}

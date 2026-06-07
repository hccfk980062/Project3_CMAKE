#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <windows.h>
#include <commdlg.h>

#include "ControlWindow.h"
#include <imgui.h>
#include <iostream>
#include <fstream>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace fs = std::filesystem;

namespace CG
{
	ControlWindow::ControlWindow()
		: showDemoWindow(false), current_texture(nullptr), targetScene(nullptr)
	{}

	auto ControlWindow::Initialize() -> bool
	{
		LoadAssetsFromJson();
		return true;
	}

	void ControlWindow::SetTargetScene(MainScene* scene)
	{
		targetScene = scene;
	}

	// Builds a StickerState from the current template and places it in the scene
	void ControlWindow::PlaceNewStickerAt(const glm::vec3& center)
	{
		if (!targetScene || !current_texture) return;

		StickerState s;
		s.name     = current_texture->name;
		s.scale    = current_texture->worldSize;
		s.repeat   = current_texture->scale;
		s.rotation = current_texture->rotationDegrees;
		s.offset   = current_texture->offset;
		s.blend     = current_texture->mix;
		s.projAxis  = current_texture->projAxis;
		s.tintColor = current_texture->tintColor;
		s.center    = center;
		s.enabled  = true;

		if (targetScene->LoadStickerTextureIntoState(current_texture->relativePath.c_str(), s))
			targetScene->AddSticker(std::move(s));
	}

	void ControlWindow::LoadAssetsFromJson()
	{
		std::ifstream file("./res/Asset.json");
		if (!file.is_open()) return;

		textures.clear();
		current_texture = nullptr;

		std::string line;
		TextureData currentTex;
		bool inObject = false;

		while (std::getline(file, line))
		{
			if (line.find("{") != std::string::npos) {
				currentTex = TextureData();
				inObject = true;
				continue;
			}
			if (!inObject) continue;

			if (line.find("\"name\":") != std::string::npos) {
				size_t first = line.find_first_of("\"", line.find(":"));
				size_t last  = line.find_last_of("\"");
				if (first != std::string::npos && last != std::string::npos && last > first)
					currentTex.name = line.substr(first + 1, last - first - 1);
			}
			else if (line.find("\"path\":") != std::string::npos) {
				size_t first = line.find_first_of("\"", line.find(":"));
				size_t last  = line.find_last_of("\"");
				if (first != std::string::npos && last != std::string::npos && last > first)
					currentTex.relativePath = line.substr(first + 1, last - first - 1);
			}
			else if (line.find("\"mix\":") != std::string::npos) {
				sscanf(line.c_str(), " %*[^:]: %f", &currentTex.mix);
			}
			else if (line.find("\"worldSize\":") != std::string::npos) {
				sscanf(line.c_str(), " %*[^:]: %f", &currentTex.worldSize);
			}
			else if (line.find("\"scale\":") != std::string::npos) {
				sscanf(line.c_str(), " %*[^[:]: [%f, %f]", &currentTex.scale.x, &currentTex.scale.y);
			}
			else if (line.find("\"rotation\":") != std::string::npos) {
				sscanf(line.c_str(), " %*[^:]: %f", &currentTex.rotationDegrees);
			}
			else if (line.find("\"offset\":") != std::string::npos) {
				sscanf(line.c_str(), " %*[^[:]: [%f, %f]", &currentTex.offset.x, &currentTex.offset.y);
			}
			else if (line.find("\"projAxis\":") != std::string::npos) {
				sscanf(line.c_str(), " %*[^:]: %d", &currentTex.projAxis);
			}
			else if (line.find("\"tintColor\":") != std::string::npos) {
				sscanf(line.c_str(), " %*[^[:]: [%f, %f, %f]",
					&currentTex.tintColor.r, &currentTex.tintColor.g, &currentTex.tintColor.b);
			}

			if (line.find("}") != std::string::npos) {
				ImTextureID texID = LoadTextureFromFile(currentTex.relativePath);
				if (texID != nullptr) {
					currentTex.id = texID;
					textures.push_back(currentTex);
				}
				else {
					std::cerr << "[ControlWindow] Failed to load: " << currentTex.relativePath << "\n";
				}
				inObject = false;
			}
		}
		file.close();

		if (!textures.empty())
			current_texture = &textures.front();
	}

	auto ControlWindow::OpenFileDialog() -> std::string
	{
		OPENFILENAMEA ofn;
		char szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize  = sizeof(ofn);
		ofn.hwndOwner    = NULL;
		ofn.lpstrFile    = szFile;
		ofn.nMaxFile     = sizeof(szFile);
		ofn.lpstrFilter  = "All Images\0*.png;*.jpg;*.jpeg;*.bmp;*.ppm\0PNG Files (*.png)\0*.png\0JPG Files (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0";
		ofn.nFilterIndex = 1;
		ofn.Flags        = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
			return std::string(ofn.lpstrFile);
		return "";
	}

	auto ControlWindow::LoadTextureFromFile(const std::string& filepath) -> ImTextureID
	{
		int width, height, channels;
		unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 4);
		if (data == nullptr) return nullptr;

		GLuint textureID;
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		stbi_image_free(data);

		return (ImTextureID)(intptr_t)textureID;
	}

	void ControlWindow::ConvertToRelativePath(const std::string& absolutePath, TextureData& tex)
	{
		try {
			tex.relativePath = fs::relative(fs::path(absolutePath), fs::current_path()).generic_string();
		}
		catch (...) {
			tex.relativePath = absolutePath;
		}
	}

	void ControlWindow::SaveAssetsToJson()
	{
		fs::create_directories("./res");
		std::ofstream file("./res/Asset.json");
		if (!file.is_open()) return;

		file << "[\n";
		for (size_t i = 0; i < textures.size(); ++i) {
			const auto& t = textures[i];
			file << "  {\n";
			file << "    \"name\": \""     << t.name             << "\",\n";
			file << "    \"path\": \""     << t.relativePath     << "\",\n";
			file << "    \"mix\": "        << t.mix              << ",\n";
			file << "    \"worldSize\": "  << t.worldSize        << ",\n";
			file << "    \"scale\": ["     << t.scale.x << ", "  << t.scale.y << "],\n";
			file << "    \"rotation\": "   << t.rotationDegrees  << ",\n";
			file << "    \"offset\": ["    << t.offset.x << ", " << t.offset.y << "],\n";
			file << "    \"projAxis\": "   << t.projAxis         << ",\n";
			file << "    \"tintColor\": [" << t.tintColor.r << ", " << t.tintColor.g << ", " << t.tintColor.b << "]\n";
			file << "  }" << (i + 1 < textures.size() ? "," : "") << "\n";
		}
		file << "]\n";
		file.close();
	}

	void ControlWindow::Display()
	{
		ImGui::Begin("Sticker Library");

		// ── Render Options ────────────────────────────────────────────
		if (targetScene)
			ImGui::Checkbox("Show Wireframe", &targetScene->showWireframe);

		ImGui::Separator();

		// ── Load / Browse ─────────────────────────────────────────────
		ImGui::InputText("Path", texturePath, sizeof(texturePath));
		ImGui::SameLine();
		if (ImGui::Button("Browse..."))
		{
			std::string selectedPath = OpenFileDialog();
			if (!selectedPath.empty())
			{
				snprintf(texturePath, sizeof(texturePath), "%s", selectedPath.c_str());
				ImTextureID texID = LoadTextureFromFile(selectedPath);
				if (texID != nullptr)
				{
					TextureData newTex;
					newTex.name = fs::path(selectedPath).stem().string();
					newTex.id   = texID;
					ConvertToRelativePath(selectedPath, newTex);
					textures.push_back(newTex);
					current_texture = &textures.back();
				}
			}
		}

		ImGui::Separator();

		// ── Asset grid ────────────────────────────────────────────────
		ImGui::Text("Asset Library:");
		ImGui::BeginChild("AssetGrid", ImVec2(0, 120), true);
		for (size_t i = 0; i < textures.size(); ++i)
		{
			ImGui::PushID(static_cast<int>(i));
			bool clicked = ImGui::ImageButton(textures[i].id, ImVec2(64, 64));

			if (current_texture == &textures[i])
			{
				ImGui::GetWindowDrawList()->AddRect(
					ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
					IM_COL32(255, 255, 0, 255), 2.0f);
			}

			if (clicked)
				current_texture = &textures[i];

			if ((i + 1) % 4 != 0 && i + 1 < textures.size()) ImGui::SameLine();
			ImGui::PopID();
		}
		ImGui::EndChild();

		// ── Placement hint ────────────────────────────────────────────
		ImGui::Separator();
		if (current_texture)
			ImGui::TextColored(ImVec4(0.4f, 0.9f, 0.4f, 1.0f),
				"Left-click on mesh to place \"%s\"", current_texture->name.c_str());
		else
			ImGui::TextDisabled("Browse and select a texture above");

		// ── Template parameters (applied to next placed sticker) ──────
		if (current_texture != nullptr)
		{
			ImGui::Separator();
			ImGui::Text("Placement Template");

			char nameBuf[64];
			snprintf(nameBuf, sizeof(nameBuf), "%s", current_texture->name.c_str());
			if (ImGui::InputText("Asset Name", nameBuf, sizeof(nameBuf)))
				current_texture->name = std::string(nameBuf);

			static const char* axisLabels[] = {
				"Camera (face viewer)",
				"+Z Front", "-Z Back", "+Y Top", "-Y Bottom", "+X Right", "-X Left"
			};
			ImGui::Combo("Projection Axis", &current_texture->projAxis, axisLabels, 7);
			ImGui::DragFloat("World Size",  &current_texture->worldSize,       0.005f, 0.01f, 2.0f);
			ImGui::DragFloat2("UV Offset",  &current_texture->offset.x,        0.01f, -2.0f, 2.0f);
			ImGui::DragFloat2("UV Tiling",  &current_texture->scale.x,         0.01f,  0.1f, 8.0f);
			ImGui::DragFloat("Rotation",    &current_texture->rotationDegrees,  1.0f, -180.0f, 180.0f);
			ImGui::DragFloat("Blend",       &current_texture->mix,             0.005f,  0.0f, 1.0f);
			ImGui::ColorEdit3("Tint Color", &current_texture->tintColor.x);

			ImGui::Spacing();

			if (ImGui::Button("Save Changes", ImVec2(120, 0)))
				SaveAssetsToJson();

			ImGui::SameLine();

			ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
			if (ImGui::Button("Delete Asset", ImVec2(120, 0)))
			{
				auto it = std::find_if(textures.begin(), textures.end(),
					[this](const TextureData& t) { return &t == current_texture; });
				if (it != textures.end())
				{
					textures.erase(it);
					current_texture = textures.empty() ? nullptr : &textures.front();
					SaveAssetsToJson();
				}
			}
			ImGui::PopStyleColor(3);
		}

		ImGui::End();
	}
}

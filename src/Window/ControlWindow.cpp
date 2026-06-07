#include <GL/glew.h>
#include <GLFW/glfw3.h>

// Windows 檔案視窗所需的標頭檔
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
	ControlWindow::ControlWindow() : showDemoWindow(false), current_texture(nullptr) {}

	auto ControlWindow::Initialize() -> bool
	{
		// 在初始化時自動嘗試讀取素材庫
		LoadAssetsFromJson();
		return true;
	}

	void ControlWindow::LoadAssetsFromJson()
	{
		std::ifstream file("./res/Asset.json");
		if (!file.is_open()) return; // 找不到檔案代表是第一次開啟，直接返回

		// 清空當前快取，避免重複載入
		textures.clear();
		current_texture = nullptr;

		std::string line;
		TextureData currentTex;
		bool inObject = false;

		// 簡易手動狀態機解析 JSON
		while (std::getline(file, line))
		{
			if (line.find("{") != std::string::npos) {
				currentTex = TextureData(); // 初始化一筆新資料
				inObject = true;
				continue;
			}

			if (!inObject) continue;

			// 解析 name
			if (line.find("\"name\":") != std::string::npos) {
				size_t first = line.find_first_of("\"", line.find(":"));
				size_t last = line.find_last_of("\"");
				if (first != std::string::npos && last != std::string::npos && last > first) {
					currentTex.name = line.substr(first + 1, last - first - 1);
				}
			}
			// 解析 path
			else if (line.find("\"path\":") != std::string::npos) {
				size_t first = line.find_first_of("\"", line.find(":"));
				size_t last = line.find_last_of("\"");
				if (first != std::string::npos && last != std::string::npos && last > first) {
					currentTex.relativePath = line.substr(first + 1, last - first - 1);
				}
			}
			// 解析 mix
			else if (line.find("\"mix\":") != std::string::npos) {
				sscanf(line.c_str(), " %*[^:]: %f", &currentTex.mix);
			}
			// 解析 scale
			else if (line.find("\"scale\":") != std::string::npos) {
				sscanf(line.c_str(), " %*[^[:]: [%f, %f]", &currentTex.scale.x, &currentTex.scale.y);
			}
			// 解析 rotation
			else if (line.find("\"rotation\":") != std::string::npos) {
				sscanf(line.c_str(), " %*[^:]: %f", &currentTex.rotationDegrees);
			}
			// 解析 offset
			else if (line.find("\"offset\":") != std::string::npos) {
				sscanf(line.c_str(), " %*[^[:]: [%f, %f]", &currentTex.offset.x, &currentTex.offset.y);
			}

			// 物件結束，開始載入貼圖並存入 vector
			if (line.find("}") != std::string::npos) {
				// 嘗試以相對路徑載入圖片（因為工作目錄是在 exe 端，res/Asset.json 寫的也是相對 exe 的路徑）
				ImTextureID texID = LoadTextureFromFile(currentTex.relativePath);
				if (texID != nullptr) {
					currentTex.id = texID;
					textures.push_back(currentTex);
				}
				else {
					std::cerr << "[ControlWindow] Failed to load texture: " << currentTex.relativePath << std::endl;
				}
				inObject = false;
			}
		}
		file.close();

		// 預設將選取目標移到第一張圖片
		if (!textures.empty()) {
			current_texture = &textures.front();
		}
	}

	// 核心功能 1：開啟 Windows 原生檔案選擇器
	auto ControlWindow::OpenFileDialog() -> std::string
	{
		OPENFILENAMEA ofn;
		char szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(ofn));
		ofn.lStructSize = sizeof(ofn);
		ofn.hwndOwner = NULL; // 或者是你的 GLFW 視窗控制代碼 (glfwGetWin32Window)
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		// 過濾檔案格式
		ofn.lpstrFilter = "All Images\0*.png;*.jpg;*.jpeg;*.bmp;*.ppm\0PNG Files (*.png)\0*.png\0JPG Files (*.jpg;*.jpeg)\0*.jpg;*.jpeg\0";
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return std::string(ofn.lpstrFile);
		}
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
			fs::path p_exe = fs::current_path();
			fs::path p_img(absolutePath);
			tex.relativePath = fs::relative(p_img, p_exe).generic_string();
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
			file << "    \"name\": \"[" << t.name << "\",\n"; // 確保名稱欄位寫入
			file << "    \"path\": \"" << t.relativePath << "\",\n";
			file << "    \"mix\": " << t.mix << ",\n";
			file << "    \"scale\": [" << t.scale.x << ", " << t.scale.y << "],\n";
			file << "    \"rotation\": " << t.rotationDegrees << ",\n";
			file << "    \"offset\": [" << t.offset.x << ", " << t.offset.y << "]\n";
			file << "  }" << (i + 1 < textures.size() ? "," : "") << "\n";
		}
		file << "]\n";
		file.close();
	}

	void ControlWindow::Display()
	{
		ImGui::Begin("Texture Asset Library");
		{
			// 顯示選取的路徑文字框
			ImGui::InputText("Path", texturePath, sizeof(texturePath));
			ImGui::SameLine();

			// 點選按鈕打開檔案管理器
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
						// 載入時預設名稱使用原始檔名（不含副檔名）
						newTex.name = fs::path(selectedPath).stem().string();
						newTex.id = texID;

						ConvertToRelativePath(selectedPath, newTex);
						textures.push_back(newTex);
						current_texture = &textures.back();

						// 根據需求：載入時不再自動 SaveAssetsToJson()，由使用者手動保存
					}
				}
			}

			ImGui::Separator();

			// 素材庫方格
			ImGui::Text("Available Assets:");
			ImGui::BeginChild("AssetGrid", ImVec2(0, 120), true);
			{
				for (size_t i = 0; i < textures.size(); ++i)
				{
					ImGui::PushID(static_cast<int>(i));
					bool clicked = ImGui::ImageButton(textures[i].id, ImVec2(64, 64));

					if (current_texture == &textures[i])
					{
						ImGui::GetWindowDrawList()->AddRect(
							ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
							IM_COL32(255, 255, 0, 255), 2.0f
						);
					}

					if (clicked) current_texture = &textures[i];
					if ((i + 1) % 4 != 0 && i + 1 < textures.size()) ImGui::SameLine();
					ImGui::PopID();
				}
			}
			ImGui::EndChild();

			// 參數連動控制與編輯區
			if (current_texture != nullptr)
			{
				ImGui::Separator();
				ImGui::Text("Editing Asset Settings");

				// 1. 素材名稱編輯（使用內建緩衝區轉換，限制 64 字元）
				char nameBuf[64];
				snprintf(nameBuf, sizeof(nameBuf), "%s", current_texture->name.c_str());
				if (ImGui::InputText("Asset Name", nameBuf, sizeof(nameBuf)))
				{
					current_texture->name = std::string(nameBuf);
				}

				// 2. 數值微調滑桿（移除原本的自動 SaveAssetsToJson 觸發）
				ImGui::SliderFloat2("Offset", &current_texture->offset.x, -2.0f, 2.0f);
				ImGui::SliderFloat2("Repeat/Scale", &current_texture->scale.x, 0.1f, 8.0f);
				ImGui::SliderFloat("Rotation", &current_texture->rotationDegrees, -180.0f, 180.0f);
				ImGui::SliderFloat("Texture Mix", &current_texture->mix, 0.0f, 1.0f);

				ImGui::Spacing();

				// 3. 保存與刪除按鈕
				if (ImGui::Button("Save Changes", ImVec2(120, 0)))
				{
					SaveAssetsToJson();
				}

				ImGui::SameLine();

				// 為了安全起見，將刪除按鈕標色
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.5f, 0.0f, 0.0f, 1.0f));

				if (ImGui::Button("Delete Asset", ImVec2(120, 0)))
				{
					// 尋找當前指標在 vector 中的迭代器位置並刪除
					auto it = std::find_if(textures.begin(), textures.end(),
						[this](const TextureData& t) { return &t == current_texture; });

					if (it != textures.end())
					{
						textures.erase(it);
						current_texture = nullptr; // 重設選取狀態
						SaveAssetsToJson();        // 刪除後同步更新 Json
					}
				}
				ImGui::PopStyleColor(3);
			}
		}
		ImGui::End();
	}
}
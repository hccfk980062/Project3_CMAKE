#include <GL/glew.h>
#include "StickerHierarchyWindow.h"
#include <imgui.h>

namespace CG
{
	void StickerHierarchyWindow::Display()
	{
		ImGui::Begin("Sticker Hierarchy");

		if (!targetScene)
		{
			ImGui::TextDisabled("No scene connected");
			ImGui::End();
			return;
		}

		auto& stickers = targetScene->stickers;

		// ── Repositioning mode indicator ──────────────────────────────
		if (targetScene->repositioningMode)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.65f, 0.0f, 1.0f));
			ImGui::TextWrapped("Reposition mode: click on the mesh to move the selected sticker.");
			ImGui::PopStyleColor();
			if (ImGui::SmallButton("Cancel Reposition"))
				targetScene->repositioningMode = false;
			ImGui::Separator();
		}

		// ── Sticker count ─────────────────────────────────────────────
		ImGui::Text("Stickers: %d", static_cast<int>(stickers.size()));
		if (stickers.empty())
		{
			ImGui::TextDisabled("No stickers placed yet.");
			ImGui::TextDisabled("Select a texture in the Library,");
			ImGui::TextDisabled("then left-click on the model.");
			ImGui::End();
			return;
		}

		ImGui::Separator();

		// ── Sticker list ──────────────────────────────────────────────
		// Track if we need to remove a sticker after the loop (avoid erase during iteration)
		int pendingDelete = -1;

		for (int i = 0; i < static_cast<int>(stickers.size()); ++i)
		{
			auto& s = stickers[i];
			ImGui::PushID(i);

			// Visibility eye checkbox
			ImGui::Checkbox("##vis", &s.enabled);
			ImGui::SameLine();

			// Selectable label
			char label[128];
			if (s.name.empty())
				snprintf(label, sizeof(label), "Sticker %d", i);
			else
				snprintf(label, sizeof(label), "%s", s.name.c_str());

			bool isSelected = (targetScene->selectedStickerIndex == i);
			if (ImGui::Selectable(label, isSelected, ImGuiSelectableFlags_AllowDoubleClick))
				targetScene->selectedStickerIndex = i;

			// Delete button on the right
			float deleteW = 22.0f;
			ImGui::SameLine(ImGui::GetContentRegionMax().x - deleteW);
			ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f, 0.1f, 0.1f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f,  0.2f, 0.2f, 1.0f));
			if (ImGui::SmallButton("X"))
				pendingDelete = i;
			ImGui::PopStyleColor(2);

			ImGui::PopID();
		}

		if (pendingDelete >= 0)
			targetScene->RemoveSticker(pendingDelete);

		ImGui::End();
	}
}

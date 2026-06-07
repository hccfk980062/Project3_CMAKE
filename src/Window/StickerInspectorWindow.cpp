#include <GL/glew.h>
#include "StickerInspectorWindow.h"
#include <imgui.h>

namespace CG
{
	static const char* kAxisLabels[] = {
		"Camera (face viewer)",
		"+Z Front", "-Z Back",
		"+Y Top",   "-Y Bottom",
		"+X Right", "-X Left"
	};

	void StickerInspectorWindow::Display()
	{
		ImGui::Begin("Sticker Inspector");

		if (!targetScene)
		{
			ImGui::TextDisabled("No scene connected");
			ImGui::End();
			return;
		}

		int idx = targetScene->selectedStickerIndex;
		if (idx < 0 || idx >= static_cast<int>(targetScene->stickers.size()))
		{
			ImGui::TextDisabled("No sticker selected.");
			ImGui::TextDisabled("Click an entry in Sticker Hierarchy.");
			ImGui::End();
			return;
		}

		StickerState& s = targetScene->stickers[idx];

		// ── Header ───────────────────────────────────────────────────
		ImGui::Text("Sticker #%d", idx);
		ImGui::Separator();

		// Name
		char nameBuf[64];
		snprintf(nameBuf, sizeof(nameBuf), "%s", s.name.c_str());
		if (ImGui::InputText("Name", nameBuf, sizeof(nameBuf)))
			s.name = nameBuf;

		// Enable
		ImGui::Checkbox("Enabled", &s.enabled);

		ImGui::Spacing();

		// ── Reposition ───────────────────────────────────────────────
		if (targetScene->repositioningMode)
		{
			ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.8f, 0.45f, 0.0f, 1.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.6f,  0.1f, 1.0f));
			if (ImGui::Button("Cancel Reposition", ImVec2(-1, 0)))
				targetScene->repositioningMode = false;
			ImGui::PopStyleColor(2);
		}
		else
		{
			if (ImGui::Button("Set Position (click on mesh)", ImVec2(-1, 0)))
				targetScene->repositioningMode = true;
		}

		// Current center (read-only)
		ImGui::Text("Center: (%.3f, %.3f, %.3f)", s.center.x, s.center.y, s.center.z);

		ImGui::Spacing();
		ImGui::Separator();

		// ── Transform parameters (live — changes appear immediately) ─
		if (ImGui::Combo("Projection Axis", &s.projAxis, kAxisLabels, 7))
			targetScene->RefreezeProjectionVectors(s);
		ImGui::DragFloat("World Size", &s.scale,    0.005f,  0.01f,  2.0f);
		ImGui::DragFloat2("UV Offset", &s.offset.x, 0.01f,  -2.0f,  2.0f);
		ImGui::DragFloat2("UV Tiling", &s.repeat.x, 0.01f,   0.1f,  8.0f);
		ImGui::DragFloat("Rotation",   &s.rotation, 1.0f,   -180.0f, 180.0f);
		ImGui::DragFloat("Blend",      &s.blend,    0.005f,  0.0f,   1.0f);
		ImGui::ColorEdit3("Tint Color", &s.tintColor.x);

		ImGui::Spacing();
		ImGui::Separator();

		// ── Delete ───────────────────────────────────────────────────
		ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.6f, 0.1f, 0.1f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.8f, 0.2f, 0.2f, 1.0f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.5f, 0.0f, 0.0f, 1.0f));
		if (ImGui::Button("Delete This Sticker", ImVec2(-1, 0)))
			targetScene->RemoveSticker(idx);
		ImGui::PopStyleColor(3);

		ImGui::End();
	}
}

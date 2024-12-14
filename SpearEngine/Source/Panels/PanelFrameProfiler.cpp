#include "PanelFrameProfiler.h"
#include "Core/FrameProfiler.h"
#include "imgui.h"
#include <algorithm>

void PanelFrameProfiler::MakePanel()
{
	const std::vector<ProfileCategory>& rootCategories = FrameProfiler::GetData();

	ImGui::Text("Target:");
	ImGui::SameLine();
	ImGui::PushItemWidth(100.f);
	ImGui::DragInt("FPS", &fpsTarget, .2f, 10, 120);
	ImGui::PopItemWidth();
	msTarget = 1000.f / fpsTarget;
	ImGui::Text("Budget: %.5f ms", msTarget);

	ImGui::Text("Hide unknowns smaller than threshold:");
	ImGui::SameLine();
	ImGui::PushItemWidth(100.f);
	ImGui::DragFloat("ms", &unknownThreshold, .001f, 0.f, 1.f);
	ImGui::PopItemWidth();
	ImGui::SameLine();
	ImGui::Text("(%.2f%% frametime)", 100 * (unknownThreshold / msTarget));

	float totalMs{ 0.f };
	if (ImGui::BeginTable("Frame Timings", 3, ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn("Category");
		ImGui::TableSetupColumn("Time (ms)");
		ImGui::TableSetupColumn("Frame Percentage");
		ImGui::TableHeadersRow();

		float totalMs{ 0.f };
		for (const ProfileCategory& category : rootCategories)
		{
			DrawCategory(category);
			totalMs += category.durationMs;
		}

		 // Draw Unknown
		const float unknownMs = FrameProfiler::GetCurrentFrameMs() - totalMs;
		const float unknownPercentage = unknownMs / msTarget;
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::Text("Unknown");
		ImGui::TableNextColumn();
		ImGui::Text("%.5f", unknownMs);
		ImGui::TableNextColumn();
		const int unknownRed = std::clamp(100 * unknownPercentage, 0.f, 255.f);
		const int unknownGreen = std::clamp(100 * (1 - unknownPercentage), 0.f, 255.f);
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(unknownRed, unknownGreen, 0, 255));
		ImGui::Text("%.2f", unknownPercentage * 100);

		// Draw Totals
		const float totalPercentage = totalMs / msTarget;
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		const int totalRed = std::clamp(100 * totalPercentage, 0.f, 255.f);
		const int totalGreen = std::clamp(100 * (1 - totalPercentage), 0.f, 255.f);
		ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(totalRed, totalGreen, 0, 255));
		ImGui::Text("Totals");
		ImGui::TableNextColumn();
		ImGui::Text("%.5f", totalMs);
		ImGui::TableNextColumn();
		ImGui::Text("%.2f", totalPercentage * 100);

		ImGui::EndTable();
	}
}

void PanelFrameProfiler::DrawCategory(const ProfileCategory& category)
{
	ImGui::TableNextRow();
	ImGui::TableNextColumn();

	bool treeOpen = false;
	if (category.childCategories.size())
	{
		treeOpen = ImGui::TreeNodeEx(category.categoryName, ImGuiTreeNodeFlags_DefaultOpen);
	}
	else
	{
		ImGui::Text(category.categoryName);
	}
	ImGui::TableNextColumn();
	ImGui::Text("%.5f", category.durationMs);
	ImGui::TableNextColumn();

	const float framePercentage = category.durationMs / msTarget;
	const int red = std::clamp(100 * framePercentage, 0.f, 255.f);
	const int green = std::clamp(100 * (1 - framePercentage), 0.f, 255.f);
	ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(red, green, 0, 255));
	ImGui::Text("%.2f", framePercentage * 100);

	if (treeOpen)
	{
		float totalSubcategoryMs{ 0.f };
		for (const ProfileCategory& subcategory : category.childCategories)
		{
			totalSubcategoryMs += subcategory.durationMs;
			DrawCategory(subcategory);
		}

		float unknownMs = category.durationMs - totalSubcategoryMs;
		if (unknownMs > unknownThreshold)
		{
			ImGui::TableNextRow();
			ImGui::TableNextColumn();
			ImGui::Text("Unknown");
			ImGui::TableNextColumn();
			ImGui::Text("%.5f", unknownMs);
			ImGui::TableNextColumn();

			const float unknownPercentage = unknownMs / msTarget;
			const int unknownRed = std::clamp(100 * unknownPercentage, 0.f, 255.f);
			const int unknownGreen = std::clamp(100 * (1 - unknownPercentage), 0.f, 255.f);
			ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(unknownRed, unknownGreen, 0, 255));
			ImGui::Text("%.2f", unknownPercentage * 100);
		}

		ImGui::TreePop();
	}
}
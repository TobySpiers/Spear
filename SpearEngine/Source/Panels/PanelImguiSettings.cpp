#include "PanelImguiSettings.h"
#include "imgui.h"

void PanelImguiSettings::MakePanel()
{
	ImGui::SeparatorText("APPEARANCE:");
	{
		ImGui::SliderFloat("ImGui Scale", &imguiScale, 0.5f, 3.0f, "%.2f"); // Apply the scale factor ImGui::GetIO().FontGlobalScale = scale_factor;
		ImGui::GetIO().FontGlobalScale = imguiScale;

		if (ImGui::Button("Classic"))
		{
			ImGui::StyleColorsClassic();
		}
		ImGui::SameLine();
		if (ImGui::Button("Dark Mode"))
		{
			ImGui::StyleColorsDark();
		}
		ImGui::SameLine();
		if (ImGui::Button("Light Mode"))
		{
			ImGui::StyleColorsLight();
		}
	}
}

#include "PanelRaycaster.h"
#include "Raycaster.h"
#include "imgui.h"

void PanelRaycaster::MakePanel()
{
	rayConfig = Raycaster::GetConfigCopy();

	ImGui::PushItemWidth(200.f);
	ImGui::SeparatorText("SETTINGS:");
	{
		ImGui::SliderInt("Internal Resolution (X)", &rayConfig.xResolution, 10, 4000);
		ImGui::SliderInt("Internal Resolution (Y)", &rayConfig.yResolution, 10, 4000);
		ImGui::SliderFloat("Far Clip", &rayConfig.farClip, 1.f, 200.f);
		ImGui::SliderFloat("Field Of View", &rayConfig.fieldOfView, 35.f, 95.f);
		ImGui::SliderInt("Ray Encounter Limit", &rayConfig.rayEncounterLimit, 1, 50);
		ImGui::SliderFloat("2D Scale", &rayConfig.scale2D, 1.f, 200.f);
		ImGui::SliderFloat("Corrective Pixel Depth Tolerance", &rayConfig.correctivePixelDepthTolerance, 0.f, 0.1f, "%.4f");
		ImGui::Checkbox("Highlight Corrective Pixels", &rayConfig.highlightCorrectivePixels);
	}
	ImGui::SeparatorText("RENDERING:");
	{
		ImGui::Checkbox("Software Rendering", &Raycaster::m_bSoftwareRendering);
		if(Raycaster::m_bSoftwareRendering)
		{
			ImGui::SliderInt("Threads (Software Renderer)", &Raycaster::m_softwareRenderingThreads, 1, 32);
		}
	}
	ImGui::PopItemWidth();

	Raycaster::ApplyConfig(rayConfig);
}

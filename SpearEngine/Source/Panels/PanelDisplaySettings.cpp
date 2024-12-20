#include "PanelDisplaySettings.h"
#include "Core/WindowManager.h"
#include "Core/ServiceLocator.h"
#include "Graphics/ScreenRenderer.h"
#include "imgui.h"

static PanelDisplaySettings panel;

void PanelDisplaySettings::MakePanel()
{
	Spear::WindowManager& window = Spear::ServiceLocator::GetWindowManager();

	if (m_windowSize.x == -1)
	{
		m_windowSize = window.GetWindowSize();
	}

	ImGui::PushItemWidth(200.f);
	ImGui::SeparatorText("WINDOW:");
	ImGui::InputInt("Resolution (X)", &m_windowSize.x, 100, 4000);
	ImGui::InputInt("Resolution (Y)", &m_windowSize.y, 100, 4000);
	ImGui::Checkbox("Fullscreen", &m_fullscreen);
	if (ImGui::Button("Apply"))
	{
		window.SetWindowSize(m_windowSize);
		window.SetWindowFullscreenMode(m_fullscreen ? Spear::eFullscreenMode::BORDERLESS : Spear::eFullscreenMode::WINDOWED);
		Spear::ServiceLocator::GetScreenRenderer().SetInternalResolution(m_windowSize.x, m_windowSize.y);
	};
	ImGui::PopItemWidth();
}

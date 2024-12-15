#include "ImguiManager.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "WindowManager.h"
#include "ServiceLocator.h"
#include "Panels/PanelImguiSettings.h"
#include "Core/FrameProfiler.h"
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace Spear
{
	static PanelImguiSettings imguiSettingsPanel;

	ImguiPanelBase::ImguiPanelBase()
	{
		ImguiManager::RegisterPanel(this);
	}

	ImguiPanelBase::~ImguiPanelBase()
	{
		ImguiManager::DeregisterPanel(this);
	}

	ImguiManager::ImguiManager()
	{
		// ImGui Setup - pulled from ImGui sample project: example_sdl2_opengl3
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;       // Enable Multi-Viewport / Platform Windows

		ImGui::StyleColorsDark();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		// Setup Platform/Renderer backends
		const char* glsl_version = "#version 410";
		WindowManager& windowManager = ServiceLocator::GetWindowManager();
		ImGui_ImplSDL2_InitForOpenGL(&windowManager.GetWindow(), &windowManager.GetContext());
		ImGui_ImplOpenGL3_Init(glsl_version);

		// Set default scale
		ImGui::GetIO().FontGlobalScale = 1.5f;
	}

	ImguiManager::~ImguiManager()
	{
	}

	ImguiManager& ImguiManager::Get()
	{
		return ServiceLocator::GetImguiManager();
	}

	bool ImguiManager::IsImguiEnabled()
	{
		return bImguiEnabled;
	}

	void ImguiManager::SetImguiEnabled(bool bEnabled)
	{
		bImguiEnabled = bEnabled;
	}

	void ImguiManager::SetMenuBarLabel(const char* newLabel)
	{
		menuLabel = newLabel;
	}

	void ImguiManager::MakePanels()
	{
		if (!bImguiEnabled)
		{
			return;
		}
		// TODO: Maybe useful for ignoring game inputs in future when panels are interacted with
		//if (!imguiIO.WantCaptureKeyboard && !imguiIO.WantCaptureMouse)

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		std::vector<ImguiPanelBase*>& panels = GetPanelList();

		// Create topbar
		if (ImGui::BeginMainMenuBar())
		{
			// Demo window menu
			if (ImGui::BeginMenu("Engine"))
			{
				if (ImGui::BeginMenu("ImGui"))
				{
					ImGui::MenuItem("Dear ImGui Demo", nullptr, &bImguiDemoEnabled);
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}

			// Custom panel menus
			for (ImguiPanelBase* panel : panels)
			{
				if (ImGui::BeginMenu(panel->Category()))
				{
					if (panel->Subcategory() != nullptr)
					{
						if(ImGui::BeginMenu(panel->Subcategory()))
						{
							ImGui::MenuItem(panel->PanelName(), nullptr, &panel->bIsOpen);

							ImGui::EndMenu();
						}
					}
					else
					{
						ImGui::MenuItem(panel->PanelName(), nullptr, &panel->bIsOpen);
					}
					ImGui::EndMenu();
				}
			}

			// Dev Info
			{
				const float frameMs = FrameProfiler::GetCurrentFrameMs();
				std::string stringMs(std::to_string(frameMs) + "ms");
				std::string stringFps("FPS: " + std::to_string(std::min(999, static_cast<int>(1000 / frameMs))));

				const float widthFps = ImGui::CalcTextSize("FPS: 000").x;
				const float widthMs = ImGui::CalcTextSize("00.000000ms").x;
				const float widthTotal = widthFps + widthMs;

				ImGui::SameLine((ImGui::GetWindowWidth() / 2) - (widthTotal / 2) - 10);
				ImGui::TextColored(ImVec4(0.f, 0.8f, 0.f, 1.f), stringMs.c_str());
				ImGui::SameLine((ImGui::GetWindowWidth() / 2) + 10);
				ImGui::TextColored(ImVec4(0.f, 0.8f, 0.f, 1.f), stringFps.c_str());

				// User label
				if (menuLabel != nullptr)
				{
					ImGui::SameLine(ImGui::GetWindowWidth() - ImGui::CalcTextSize(menuLabel).x - 10);
					ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
					ImGui::Text(menuLabel);
					ImGui::PopStyleColor();
				}
			}

			
			ImGui::EndMainMenuBar();
		}

		// Create demo window
		if (bImguiDemoEnabled)
		{
			ImGui::ShowDemoWindow(&bImguiDemoEnabled);
		}

		// Create panels
		for (ImguiPanelBase* panel : panels)
		{
			if (panel->bIsOpen)
			{
				ImGui::SetNextWindowSize(panel->DefaultPanelSize(), ImGuiCond_Once);
				ImGui::Begin(panel->PanelName(), &panel->bIsOpen);
				panel->MakePanel();
				ImGui::End();
			}
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		// Update and Render additional Platform Windows
		// (Platform functions may change the current OpenGL context, so we save/restore it to make it easier to paste this code elsewhere.
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
			SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
			SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
		}
	}

	std::vector<ImguiPanelBase*>& ImguiManager::GetPanelList()
	{
		static std::vector<ImguiPanelBase*> m_panels;
		return m_panels;
	}

	void ImguiManager::RegisterPanel(ImguiPanelBase* panel)
	{
		GetPanelList().push_back(panel);
	}

	void ImguiManager::DeregisterPanel(ImguiPanelBase* panel)
	{
		std::vector<ImguiPanelBase*>& panels = GetPanelList();

		auto it = std::find(panels.begin(), panels.end(), panel);
		if (it != panels.end())
		{
			panels.erase(it);
		}
	}
}
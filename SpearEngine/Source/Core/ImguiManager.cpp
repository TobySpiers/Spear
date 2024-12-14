#include "ImguiManager.h"
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include "WindowManager.h"
#include "ServiceLocator.h"
#include "Panels/PanelImguiSettings.h"
#include <algorithm>

namespace Spear
{
	//std::vector<ImguiPanelBase*> ImguiManager::m_panels;
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

	void ImguiManager::SetPanelsEnabled(bool bEnabled)
	{
		bImguiEnabled = bEnabled;
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
				//ImGui::SetNextWindowSize(panel->DefaultPanelSize(), ImGuiCond_FirstUseEver);
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
#pragma once
#include "Core.h"

namespace Spear
{
	class ImguiPanelBase
	{
	public:
		ImguiPanelBase();
		virtual ~ImguiPanelBase();

		virtual const char* Category() const = 0;
		virtual const char* Subcategory() const { return nullptr; };
		virtual const char* PanelName() const = 0;
		virtual void MakePanel() = 0;

		virtual const Vector2i DefaultPanelSize() { return { 300, 600 }; };

	protected:
		friend class ImguiManager;
		bool bIsOpen = false;
	};

	class ImguiManager
	{
	public:
		NO_COPY(ImguiManager);

		ImguiManager();
		~ImguiManager();

		static ImguiManager& Get();

		void SetPanelsEnabled(bool bEnabled);

		// Register a panel against an associated user-defined category ID
		static void RegisterPanel(ImguiPanelBase* panel);
		// Deregister an individual panel
		static void DeregisterPanel(ImguiPanelBase* panel);

	private:
		friend class Core;
		friend class ImguiPanelBase;

		void MakePanels();
		static std::vector<ImguiPanelBase*>& GetPanelList();

		bool bImguiEnabled = true;
		bool bImguiDemoEnabled = false;

		bool bConsumeMouse = true;
		bool bConsumeKeyboard = true;
	};
}

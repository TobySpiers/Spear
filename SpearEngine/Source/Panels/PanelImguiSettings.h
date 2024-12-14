#pragma once
#include "Core/ImguiManager.h"

class PanelImguiSettings : public Spear::ImguiPanelBase
{
	virtual const char* Category() const override { return "Engine"; };
	virtual const char* Subcategory() const override { return "ImGui"; };
	virtual const char* PanelName() const override { return "Settings"; };
	virtual void MakePanel() override;

private:
	float imguiScale{ 1.5f };
};

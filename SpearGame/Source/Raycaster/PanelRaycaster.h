#pragma once
#include "Core/ImguiManager.h"
#include "RaycasterConfig.h"

class PanelRaycaster : public Spear::ImguiPanelBase
{
	virtual const char* Category() const override { return "Game"; };
	virtual const char* PanelName() const override { return "Raycaster"; };
	virtual const Vector2i DefaultPanelSize() override { return { 600, 400 }; };
	virtual void MakePanel() override;

private:
	RaycasterConfig rayConfig;
};

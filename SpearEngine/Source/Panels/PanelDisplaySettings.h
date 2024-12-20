#pragma once
#include "Core/ImguiManager.h"

class PanelDisplaySettings : public Spear::ImguiPanelBase
{
	virtual const char* Category() const override { return "Engine"; };
	virtual const char* PanelName() const override { return "Window"; };
	virtual const Vector2i DefaultPanelSize() override { return { 400, 200 }; };
	virtual void MakePanel() override;

private:
	Vector2i m_windowSize{ -1, -1 };
	bool m_fullscreen{ false };
};


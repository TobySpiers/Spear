#pragma once
#include "Core/ImguiManager.h"

struct ProfileCategory;

class PanelFrameProfiler : public Spear::ImguiPanelBase
{
	virtual const char* Category() const override { return "Engine"; };
	virtual const char* Subcategory() const override { return "Stats"; };
	virtual const char* PanelName() const override { return "Frame Profiler"; };
	virtual const Vector2i DefaultPanelSize() override { return { 900, 600 }; };
	virtual void MakePanel() override;

private:
	void DrawCategory(const ProfileCategory& category);
	int fpsTarget{ 60 };
	float msTarget{ 0.f };
	float unknownThreshold{ 0.f };
};
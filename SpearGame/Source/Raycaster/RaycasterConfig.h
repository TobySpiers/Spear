#pragma once

struct RaycasterConfig
{
	// CAUTION - CHANGES MADE TO THIS STRUCT MUST BE REFLECTED IN RAYCASTER COMPUTE SHADER
	// ===================================================================================

	float fieldOfView{ 80.f };
	float farClip{ 50 };
	int xResolution{ 1280 }; // internal resolution for raycaster
	int yResolution{ 720 };
	int rayEncounterLimit{ 24 }; // how many 'wall encounters' to allow per ray (used for rendering tall walls behind shorter walls)

	// Used only for 2D top-down rendering. Scale 1 = 1 tile : 1 pixel.
	float scale2D{ 75.f };

	// Debug settings
	bool highlightCorrectivePixels{ false };		// whether to render corrective pixels as BrightRed instead of using pixel-cloning
	float correctivePixelDepthTolerance{ 0.01f };	// depth tolerance for considering other pixels when stitching seams together
};
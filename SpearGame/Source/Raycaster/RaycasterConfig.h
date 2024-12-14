#pragma once

struct RaycasterConfig
{
	float fieldOfView{ 75.f };
	float farClip{ 50 };
	int xResolution{ 900 }; // internal resolution for raycaster
	int yResolution{ 600 };
	int threads{ 15 };
	int rayEncounterLimit{ 20 }; // how many 'wall encounters' to allow per ray (used for rendering tall walls behind shorter walls)
	int XResolutionPerThread() { return xResolution / threads; }; // intentional integer division
	int YResolutionPerThread() { return yResolution / threads; };

	// Used only for 2D top-down rendering. Scale 1 = 1 tile : 1 pixel.
	float scale2D{ 75.f };

	// Debug settings
	bool highlightCorrectivePixels{ false };		// whether to render corrective pixels as BrightRed instead of using pixel-cloning
	float correctivePixelDepthTolerance{ 0.01f };	// depth tolerance for considering other pixels when stitching seams together
};
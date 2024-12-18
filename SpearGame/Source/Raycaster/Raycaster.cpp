#include "Core/Core.h"
#include "Core/ServiceLocator.h"
#include "Core/ThreadManager.h"
#include "Core/FrameProfiler.h"
#include "Graphics/ScreenRenderer.h"
#include "Graphics/TextureArray.h"

#include "Raycaster.h"
#include "RaycasterConfig.h"
#include "LevelFileManager.h"
#include <algorithm>

PanelRaycaster Raycaster::debugPanel;

GLuint* Raycaster::m_bgTexRGBA{nullptr};
GLfloat* Raycaster::m_bgTexDepth{nullptr};
MapData* Raycaster::m_map{ nullptr };
RaycasterConfig Raycaster::m_rayConfig;
GLfloat Raycaster::m_mapMaxDepth{FLT_MAX};
Raycaster::RaycastFrameData Raycaster::m_frame;

constexpr float FOV_MIN{ 35.f };
constexpr float FOV_MAX{ 95.f };

void Raycaster::RecreateBackgroundArrays(int width, int height)
{
	delete[] m_bgTexRGBA;
	m_bgTexRGBA = new GLuint[width * height];
	std::fill(m_bgTexRGBA, m_bgTexRGBA + (width * height), GLuint(0));

	delete[] m_bgTexDepth;
	m_bgTexDepth = new GLfloat[width * height];
	std::fill(m_bgTexDepth, m_bgTexDepth + (width * height), GLfloat(m_mapMaxDepth));
}

void Raycaster::ClearBackgroundArrays()
{
	std::fill(m_bgTexRGBA, m_bgTexRGBA + (m_rayConfig.xResolution * m_rayConfig.yResolution), GLuint(0));
	std::fill(m_bgTexDepth, m_bgTexDepth + (m_rayConfig.xResolution * m_rayConfig.yResolution), GLfloat(m_mapMaxDepth));
}

void Raycaster::Init(MapData& map)
{
	if (m_bgTexRGBA == nullptr)
	{
		RecreateBackgroundArrays(m_rayConfig.xResolution, m_rayConfig.yResolution);
	}
	m_mapMaxDepth = sqrt(pow(map.gridWidth, 2) + pow(map.gridHeight, 2));

	m_map = &map;

	ApplyConfig(m_rayConfig);
}

RaycasterConfig Raycaster::GetConfigCopy()
{
	return m_rayConfig;
}

void Raycaster::ApplyConfig(const RaycasterConfig& config)
{ 
	if (config.xResolution != m_rayConfig.xResolution || config.yResolution != m_rayConfig.yResolution)
	{
		RecreateBackgroundArrays(config.xResolution, config.yResolution);
	}

	m_rayConfig = config;
	m_rayConfig.fieldOfView = std::clamp(m_rayConfig.fieldOfView, FOV_MIN, FOV_MAX);
	ApplyFovModifier(0.f);
}

void Raycaster::ApplyFovModifier(float fovModifier)
{
	// Calculate new fov
	const float fovModMin{FOV_MIN - m_rayConfig.fieldOfView};
	const float fovModMax{FOV_MAX - m_rayConfig.fieldOfView};
	const float fovMod = std::clamp(fovModifier, fovModMin, fovModMax);
	const float fovResult = m_rayConfig.fieldOfView + fovMod;

	// FoV naturally affects floor/ceiling height but not wall height... This LUT approximates sensible wall height adjustments
	// There is probably a simple formula to express this as a curve but the current approach works well enough
	// Perhaps not an ideal solution but solves the problem with a negligible performance cost
	// Maybe something to revisit at a later date
	constexpr int fovWallHeightLUTSize{ 9 };
	const Vector2f fovWallHeightLUT[fovWallHeightLUTSize] = {
		{35.f, 1.19f},
		{45.f, 1.16f},
		{50.f, 1.13f},
		{55.f, 1.11f},
		{62.5f, 1.07f},
		{75.f, 1.f},
		{90.f, 0.888f},
		{105.f, 0.777f},
		{120.f, 0.63f},
	};

	// Update cached data
	m_frame.fovWallMultiplier = Lerp(fovResult, fovWallHeightLUT, fovWallHeightLUTSize);
	m_frame.fov = TO_RADIANS(fovResult);
}

Vector2i Raycaster::GetResolution()
{
	return Vector2i(m_rayConfig.xResolution, m_rayConfig.yResolution);
}

void Raycaster::Draw2DGrid(const Vector2f& pos, const float angle)
{
	Spear::ScreenRenderer& rend = Spear::ServiceLocator::GetScreenRenderer();
	ClearBackgroundArrays();
	rend.SetBackgroundTextureDataRGBA(m_bgTexRGBA, m_bgTexDepth, m_rayConfig.xResolution, m_rayConfig.yResolution);

	constexpr float opacityFloor = 0.5f;
	constexpr float opacityRoof = 0.7f;
	constexpr float depthWorld = 0.5f;
	constexpr float depthPlayer = 0.25f;

	// Draw player
	Spear::ScreenRenderer::LinePolyData playerPoly;
	playerPoly.colour = Colour4f::Red();
	playerPoly.radius = 0.2f * m_rayConfig.scale2D;
	playerPoly.segments = 3;
	playerPoly.pos = Spear::Core::GetWindowSize().ToFloat() / 2;
	playerPoly.rotation = angle;
	playerPoly.depth = depthPlayer;
	rend.AddLinePoly(playerPoly);

	const Vector2f camOffset = (Spear::Core::GetWindowSize().ToFloat() / 2) - pos * m_rayConfig.scale2D;

	// Draw tiles
	for (int x = 0; x < m_map->gridWidth; x++)
	{
		for (int y = 0; y < m_map->gridHeight; y++)
		{
			const int nodeIndex = x + (y * m_map->gridWidth);
			GridNode& node = m_map->pNodes[nodeIndex];
			bool bWallTexture = node.texIdWall != TEX_NONE;
			bool bRoofTexture = node.texIdRoof[0] != TEX_NONE;
			int texId = bWallTexture ? node.texIdWall : node.texIdFloor[0];
			if (texId != TEX_NONE)
			{
				Spear::ScreenRenderer::SpriteData sprite;
				sprite.pos = Vector2f(x, y) * m_rayConfig.scale2D;
				sprite.pos += (Vector2f(m_rayConfig.scale2D, m_rayConfig.scale2D) / 2) + camOffset;
				sprite.size *= 1.2f;
				sprite.texLayer = texId;
				if (!bWallTexture)
				{
					if(bRoofTexture)
					{
						sprite.opacity = opacityRoof;
					}
					else
					{
						sprite.opacity = opacityFloor;
					}
				}
				if (!bRoofTexture)
				{
					sprite.depth = depthWorld;
				}

				rend.AddSprite(sprite, 0);
			}
		}
	}

	// Draw rays
	const Vector2f forward{ Normalize(Vector2f(cos(angle), sin(angle))) };

	const float halfFov{ m_frame.fov / 2 };
	Vector2f fovLeftExtent{ pos + (Vector2f(cos(angle - halfFov), sin(angle - halfFov)) * m_rayConfig.farClip) };
	Vector2f fovRightExtent{ pos + (Vector2f(cos(angle + halfFov), sin(angle + halfFov)) * m_rayConfig.farClip) };
	const Vector2f fovExtentWidth{ fovRightExtent - fovLeftExtent };

	const float raySpacing{ fovExtentWidth.Length() / m_rayConfig.xResolution };
	const Vector2f raySpacingDir{ forward.Normal() * -1 };

	// Using DDA (digital differential analysis) to quickly calculate intersections
	for(int x = 0; x < m_rayConfig.xResolution; x++)
	{
		Vector2f rayStart = pos;
		Vector2f rayEnd{ fovLeftExtent - (raySpacingDir * raySpacing * x) };
		Vector2f rayDir = Normalize(rayEnd - rayStart);

		Vector2f rayUnitStepSize{ sqrt(1 + (rayDir.y / rayDir.x) * (rayDir.y / rayDir.x)),		// length required to travel 1 X unit in Ray Direction
									sqrt(1 + (rayDir.x / rayDir.y) * (rayDir.x / rayDir.y)) };	// length required to travel 1 Y unit in Ray Direction

		Vector2i mapCheck = rayStart.ToInt(); // truncation will 'snap' position to tile
		Vector2f rayLength1D; // total length of ray: via x units, via y units
		Vector2i step;

		// ====================================
		// PREPARE INITIAL RAY LENGTH/DIRECTION
		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		if (rayDir.x < 0)
		{
			// going left
			step.x = -1;

			// calculate ray length needed to reach left edge. Example: rayStart position of (7.33) - 7 leaves us with: 0.33 (ie. we are 33% across this tile)
			// 0.33 * rayUnitStepsize = length of the ray to travel 1 unit in X scaled to the actual distance from edge
			rayLength1D.x = (rayStart.x - static_cast<float>(mapCheck.x)) * rayUnitStepSize.x; 
			
		}
		else
		{
			// going right
			step.x = 1;

			// calculate length to reach right edge. Example: 8 - position (7.33) = 0.66, multiplied by length of 1 x unit in ray direction
			rayLength1D.x = (static_cast<float>(mapCheck.x + 1) - rayStart.x) * rayUnitStepSize.x;
		}
		if (rayDir.y < 0)
		{
			step.y = -1;
			rayLength1D.y = (rayStart.y - static_cast<float>(mapCheck.y)) * rayUnitStepSize.y;
		}
		else
		{
			step.y = 1;
			rayLength1D.y = (static_cast<float>(mapCheck.y + 1) - rayStart.y) * rayUnitStepSize.y;
		}

		// ====================================
		// DETERMINE RAY LENGTH
		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		bool tileFound{false};
		float distance{0.f};
		while (!tileFound && distance < m_rayConfig.farClip)
		{
			if (rayLength1D.x < rayLength1D.y)
			{
				// X distance is currently shortest, increase X
				mapCheck.x += step.x;
				distance = rayLength1D.x;
				rayLength1D.x += rayUnitStepSize.x; // increase ray by 1 X unit
			}
			else
			{
				// Y distance is currently shortest, increase Y
				mapCheck.y += step.y;
				distance = rayLength1D.y;
				rayLength1D.y += rayUnitStepSize.y; // increase ray by 1 Y unit
			}

			// Check position is within range of array
			if(mapCheck.x >= 0 && mapCheck.x < m_map->gridWidth && mapCheck.y >= 0 && mapCheck.y < m_map->gridHeight)
			{	
				// if tile is assigned a tex value it EXISTS
				if (m_map->pNodes[mapCheck.x + (mapCheck.y * m_map->gridWidth)].texIdWall != eLevelTextures::TEX_NONE)
				{
					tileFound = true;
				}
			}
		}

		// ====================================
		// RENDER
		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		Spear::ScreenRenderer::LineData line;
		if (tileFound)
		{
			rayEnd = rayStart + rayDir * distance;
			line.colour = Colour4f::Red();
		}
		line.start = (pos * m_rayConfig.scale2D) + camOffset;
		line.end = (rayEnd * m_rayConfig.scale2D) + camOffset;
		line.depth = depthPlayer;
		rend.AddLine(line);
	}

	//FLOOR CASTING
	for (int y = m_rayConfig.yResolution / 2 + 1; y < m_rayConfig.yResolution; y++)
	{
		// Current y position compared to the center of the screen (the horizon)
		// Starts at 1, increases to HalfHeight
		int p = y - m_rayConfig.yResolution / 2;

		// Vertical position of the camera.
		float posZ = 0.61f * m_rayConfig.yResolution;

		// Horizontal distance from the camera to the floor for the current row.
		// 0.5 is the z position exactly in the middle between floor and ceiling.
		float rowDistance = posZ / p;

		// unit vectors representing directions to form left/right edge of FoV
		Vector2f planeDirLeft = Vector2f(cos(angle - halfFov), sin(angle - halfFov));
		Vector2f planeDirRight = Vector2f(cos(angle + halfFov), sin(angle + halfFov));

		// starting position of first pixel in row (left)
		// essentially the 'left-most ray' along a length (depth) of rowDistance
		Vector2f floorRayEnd = pos + rowDistance * planeDirLeft;

		// vector representing position offset equivalent to 1 pixel right (imagine topdown 2D view, this 'jumps' over by 1 ray)
		Vector2f floorRayStep = rowDistance * (planeDirRight - planeDirLeft) / m_rayConfig.xResolution;

		for (int x = 0; x < m_rayConfig.xResolution; ++x)
		{
			int cellX = (int)(floorRayEnd.x);
			int cellY = (int)(floorRayEnd.y);

			// If we render floorX and floorY to the screen, we can view the positions on the 2D grid each floor pixel samples its texture from!
			//ScreenRenderer::LinePolyData marker;
			//marker.colour = Colour4f::Red();
			//marker.radius = 5.f;
			//marker.pos = Vector2f(floorX, floorY) * m_rayConfig.scale2D;
			//marker.segments = 6;
			//rend.AddLinePoly(marker);

			floorRayEnd += floorRayStep;
		}
	}
}

void Raycaster::Draw3DGrid(const Vector2f& inPos, float inPitch, const float angle)
{
	Spear::ScreenRenderer& renderer = Spear::ServiceLocator::GetScreenRenderer();
	Spear::ThreadManager& threader = Spear::ServiceLocator::GetThreadManager();

	// Calculate const data for this frame
	{
		// Calculate 'real' pitch (in_pitch is just a percentage from -1 to +1)
		m_frame.viewPitch = std::roundf(inPitch * m_rayConfig.yResolution);
		m_frame.viewHeight = 0.625f * m_rayConfig.yResolution;
		m_frame.viewForward = Normalize(Vector2f(cos(angle), sin(angle)));
		m_frame.viewPos = inPos;

		// Screen Plane ray-distribution to avoid squash/stretch warping
		const float halfFov{ m_frame.fov / 2 };
		m_frame.screenPlaneEdgePositionL = inPos + (Vector2f(cos(angle - halfFov), sin(angle - halfFov)) * m_rayConfig.farClip);
		m_frame.screenPlaneEdgePositionR = inPos + (Vector2f(cos(angle + halfFov), sin(angle + halfFov)) * m_rayConfig.farClip);
		m_frame.screenPlaneVector = m_frame.screenPlaneEdgePositionR - m_frame.screenPlaneEdgePositionL;

		// Ray angle/spacing data
		m_frame.raySpacingDir = m_frame.viewForward.Normal() * -1; // 'Normal' as in 'perpendicular': gives us sideways direction
		m_frame.raySpacingLength = m_frame.screenPlaneVector.Length() / m_rayConfig.xResolution;

		// unit vectors representing directions to form left/right edge of FoV
		m_frame.fovMinAngle = Vector2f(cos(angle - halfFov), sin(angle - halfFov));
		m_frame.fovMaxAngle = Vector2f(cos(angle + halfFov), sin(angle + halfFov));		
	}

	// Floor/Ceiling Casting
	auto RaycastPlanesTask = [](int taskId)
	{
		const Spear::TextureBase* pMapTextures = Spear::ServiceLocator::GetScreenRenderer().GetBatchTextures(0);

		int yLowerBound = m_rayConfig.YResolutionPerThread() * taskId;
		int yUpperBound = taskId == m_rayConfig.threads - 1 ? m_rayConfig.yResolution : yLowerBound + m_rayConfig.YResolutionPerThread(); // avoid skipping pixels under non-perfect division

		for (int y = yLowerBound; y < yUpperBound; y++) // for the chunk of Y pixels allocated to this thread...
		{
			// Current y position compared to the center of the screen (the horizon)
			// Starts at 1, increases to HalfHeight
			const int rayPitchFloor = (y - (m_rayConfig.yResolution / 2)) + m_frame.viewPitch;
			const int rayPitchRoof = (y - (m_rayConfig.yResolution / 2)) - m_frame.viewPitch;

			// Horizontal distance from the camera to the floor for the current row.
			// 0.5 is the z position exactly in the middle between floor and ceiling.
			const float rowDistanceFloor = m_frame.viewHeight / rayPitchFloor;
			const float rowDistanceFloorDouble = rowDistanceFloor * 2;
			const float rowDistanceRoof = m_frame.viewHeight / rayPitchRoof;
			const float rowDistanceRoofDouble = rowDistanceRoof * 2;

			// vector representing position offset equivalent to 1 pixel right (imagine topdown 2D view, this 'jumps' horizontally by 1 ray)
			Vector2f floorStep[2];
			floorStep[0] = rowDistanceFloor * (m_frame.fovMaxAngle - m_frame.fovMinAngle) / m_rayConfig.xResolution;
			floorStep[1] = floorStep[0] * 2;
			Vector2f roofStep[2];
			roofStep[0] = rowDistanceRoof * (m_frame.fovMaxAngle - m_frame.fovMinAngle) / m_rayConfig.xResolution;
			roofStep[1] = roofStep[0] * 2;

			// endpoint of first ray in row (left)
			// essentially the 'left-most ray' along a length (depth) of rowDistance
			Vector2f rayEndFloor[2];
			rayEndFloor[0] = m_frame.viewPos + rowDistanceFloor * m_frame.fovMinAngle;
			rayEndFloor[1] = m_frame.viewPos + rowDistanceFloorDouble * m_frame.fovMinAngle;
			Vector2f rayEndRoof[2];
			rayEndRoof[0] = m_frame.viewPos + rowDistanceRoof * m_frame.fovMinAngle;
			rayEndRoof[1] = m_frame.viewPos + rowDistanceRoofDouble * m_frame.fovMinAngle;

			// calculate 'depth' for this strip of floor (same depth will be shared for all drawn pixels in one row of X)
			Vector2f rayStartFloor[2];
			rayStartFloor[0] = m_frame.viewPos + Projection(rowDistanceFloor * m_frame.fovMinAngle, m_frame.viewForward.Normal());
			rayStartFloor[1] = m_frame.viewPos + Projection(rowDistanceFloorDouble * m_frame.fovMinAngle, m_frame.viewForward.Normal());
			Vector2f rayStartRoof[2];
			rayStartRoof[0] = m_frame.viewPos + Projection(rowDistanceRoof * m_frame.fovMinAngle, m_frame.viewForward.Normal());
			rayStartRoof[1] = m_frame.viewPos + Projection(rowDistanceRoofDouble * m_frame.fovMinAngle, m_frame.viewForward.Normal());

			float stripDepthFloor[2];
			stripDepthFloor[0] = (rayEndFloor[0] - rayStartFloor[0]).Length() / m_mapMaxDepth;
			stripDepthFloor[1] = (rayEndFloor[1] - rayStartFloor[1]).Length() / m_mapMaxDepth;
			float stripDepthRoof[2];
			stripDepthRoof[0] = (rayEndRoof[0] - rayStartRoof[0]).Length() / m_mapMaxDepth;
			stripDepthRoof[1] = (rayEndRoof[1] - rayStartRoof[1]).Length() / m_mapMaxDepth;

			// Indexes into texture/depth arrays for the start of the row equal to Y value
			const int rowIndexFloor = (m_rayConfig.yResolution - (y + 1)) * m_rayConfig.xResolution;
			const int rowIndexRoof = y * m_rayConfig.xResolution;

			// Draw background texture
			for (int x = 0; x < m_rayConfig.xResolution; ++x)
			{
				// Prevent drawing floor textures behind camera
				if (rowDistanceFloor > 0)
				{
					for (int layer = 0; layer < 2; layer++)
					{
						// Render floor
						const int floorCellX = (int)(rayEndFloor[layer].x);
						const int floorCellY = (int)(rayEndFloor[layer].y);

						if (const GridNode* floorNode = m_map->GetNode(floorCellX, floorCellY))
						{
							// Floor tex sampling
							if (floorNode->texIdFloor[layer] != eLevelTextures::TEX_NONE)
							{
								const SDL_Surface* pFloorTexture = pMapTextures->GetSDLSurface(floorNode->texIdFloor[layer]);
								ASSERT(pFloorTexture);

								int texX = static_cast<int>((rayEndFloor[layer].x - floorCellX) * pFloorTexture->w);
								int texY = static_cast<int>((rayEndFloor[layer].y - floorCellY) * pFloorTexture->h);
								if (texX < 0)
									texX += pFloorTexture->w;
								if (texY < 0)
									texY += pFloorTexture->h;

								ASSERT(texX < pFloorTexture->w);
								ASSERT(texY < pFloorTexture->h);
								Uint32* pixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pFloorTexture->pixels) + (texY * pFloorTexture->pitch) + (texX * pFloorTexture->format->BytesPerPixel));
								Uint8 r, g, b, a;
								SDL_GetRGBA(*pixel, pFloorTexture->format, &r, &g, &b, &a);

								const int textureArrayIndex{ rowIndexFloor + x };
								GLuint& colByte = m_bgTexRGBA[textureArrayIndex];
								colByte |= (r << 0);
								colByte |= (g << 8);
								colByte |= (b << 16);
								colByte |= (a << 24);

								m_bgTexDepth[textureArrayIndex] = stripDepthFloor[layer];

								// We're drawing the floors nearest-first, so break this inner for-loop as soon as we draw a pixel (ie. no need to calculate pixels BEHIND this)
								break;
							}
						}
					}
					rayEndFloor[0] += floorStep[0];
					rayEndFloor[1] += floorStep[1];
				}

				// Prevent drawing roof textures behind camera
				if (rowDistanceRoof > 0)
				{
					for (int layer = 0; layer < 2; layer++)
					{
						// Render roof
						const int roofCellX = (int)(rayEndRoof[layer].x);
						const int roofCellY = (int)(rayEndRoof[layer].y);

						if (const GridNode* roofNode = m_map->GetNode(roofCellX, roofCellY))
						{
							// Roof tex sampling
							if (roofNode->texIdRoof[layer] != eLevelTextures::TEX_NONE)
							{
								const SDL_Surface* pRoofTexture = pMapTextures->GetSDLSurface(roofNode->texIdRoof[layer]);
								ASSERT(pRoofTexture);

								int texX = static_cast<int>((rayEndRoof[layer].x - roofCellX) * pRoofTexture->w);
								int texY = static_cast<int>((rayEndRoof[layer].y - roofCellY) * pRoofTexture->h);
								if (texX < 0)
									texX += pRoofTexture->w;
								if (texY < 0)
									texY += pRoofTexture->h;

								ASSERT(texX < pRoofTexture->w);
								ASSERT(texY < pRoofTexture->h);
								Uint32* pixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pRoofTexture->pixels) + (texY * pRoofTexture->pitch) + (texX * pRoofTexture->format->BytesPerPixel));
								Uint8 r, g, b, a;
								SDL_GetRGBA(*pixel, pRoofTexture->format, &r, &g, &b, &a);

								const int textureArrayIndex{ rowIndexRoof + x };
								GLuint& colByte = m_bgTexRGBA[textureArrayIndex];
								colByte |= (r << 0);
								colByte |= (g << 8);
								colByte |= (b << 16);
								colByte |= (a << 24);

								m_bgTexDepth[textureArrayIndex] = stripDepthRoof[layer];

								// We're drawing the roofs nearest-first, so break this inner for-loop as soon as we draw a pixel (ie. no need to calculate pixels BEHIND this)
								break;
							}
						}
					}
					rayEndRoof[0] += roofStep[0];
					rayEndRoof[1] += roofStep[1];
				}
			}
		}

		return 0;
	};
	START_PROFILE("Raycast Planes")
	Spear::TaskHandle RaycastPlanesTaskHandle;
	threader.DispatchTaskDistributed(RaycastPlanesTask, &RaycastPlanesTaskHandle, m_rayConfig.threads);
	RaycastPlanesTaskHandle.WaitForTaskComplete();
	END_PROFILE("Raycast Planes")

	// Using DDA (digital differential analysis) to quickly calculate intersections
	auto RaycastWallsTask = [](int taskId)
	{
		const Spear::TextureBase* pMapTextures = Spear::ServiceLocator::GetScreenRenderer().GetBatchTextures(0);

		int xLowerBound = m_rayConfig.XResolutionPerThread() * taskId;
		int xUpperBound = taskId == m_rayConfig.threads - 1 ? m_rayConfig.xResolution : xLowerBound + m_rayConfig.XResolutionPerThread(); // avoid skipping pixels under non-perfect division

		for (int screenX = xLowerBound; screenX < xUpperBound; screenX++) // for the chunk of X pixels allocated to this thread...
		{
			Vector2f rayStart = m_frame.viewPos;
			Vector2f rayEnd{ m_frame.screenPlaneEdgePositionL - (m_frame.raySpacingDir * m_frame.raySpacingLength * screenX) };
			Vector2f rayDir = Normalize(rayEnd - rayStart);

			Vector2f rayUnitStepSize{ sqrt(1 + (rayDir.y / rayDir.x) * (rayDir.y / rayDir.x)),		// length required to travel 1 X unit in Ray Direction
										sqrt(1 + (rayDir.x / rayDir.y) * (rayDir.x / rayDir.y)) };	// length required to travel 1 Y unit in Ray Direction

			Vector2i mapCheck = rayStart.ToInt(); // truncation will 'snap' position to tile
			Vector2f rayLength1D; // total length of ray: via x units, via y units
			Vector2i step;

			// ====================================
			// PREPARE INITIAL RAY LENGTH/DIRECTION
			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			if (rayDir.x < 0)
			{
				// going left
				step.x = -1;

				// calculate ray length needed to reach left edge. Example: rayStart position of (7.33) - 7 leaves us with: 0.33 (ie. we are 33% across this tile)
				// 0.33 * rayUnitStepsize = length of the ray to travel 1 unit in X, scaled by the actual distance from edge
				rayLength1D.x = (rayStart.x - static_cast<float>(mapCheck.x)) * rayUnitStepSize.x;
			}
			else
			{
				// going right
				step.x = 1;

				// calculate length to reach right edge. Example: 8 - position (7.33) = 0.66, multiplied by length of 1 x unit in ray direction
				rayLength1D.x = (static_cast<float>(mapCheck.x + 1) - rayStart.x) * rayUnitStepSize.x;
			}
			if (rayDir.y < 0)
			{
				step.y = -1;
				rayLength1D.y = (rayStart.y - static_cast<float>(mapCheck.y)) * rayUnitStepSize.y;
			}
			else
			{
				step.y = 1;
				rayLength1D.y = (static_cast<float>(mapCheck.y + 1) - rayStart.y) * rayUnitStepSize.y;
			}

			// ====================================
			// DETERMINE RAY LENGTH
			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			int rayEncounters{ 0 }; // track how many walls this ray has encountered (used to render tall walls behind short walls)
			eRayHit rayHit{ RAY_NOHIT };
			float distance{ 0.f };
			int wallNodeIndex{ 0 };
			while (rayEncounters < m_rayConfig.rayEncounterLimit && distance < m_rayConfig.farClip)
			{
				rayHit = RAY_NOHIT;
				while (rayHit == RAY_NOHIT && distance < m_rayConfig.farClip)
				{
					bool side{ false };
					if (rayLength1D.x < rayLength1D.y)
					{
						// X distance is currently shortest, increase X
						mapCheck.x += step.x;
						distance = rayLength1D.x;
						rayLength1D.x += rayUnitStepSize.x; // increase ray by 1 X unit

						side = true;
					}
					else
					{
						// Y distance is currently shortest, increase Y
						mapCheck.y += step.y;
						distance = rayLength1D.y;
						rayLength1D.y += rayUnitStepSize.y; // increase ray by 1 Y unit
					}

					// Check position is within range of array
					if (const GridNode* node = m_map->GetNode(mapCheck))
					{
						wallNodeIndex = mapCheck.x + (mapCheck.y * m_map->gridWidth);

						// if tile has a wall texture and is tall enough to be visible...
						const bool wallExists = node->texIdWall != TEX_NONE || (node->extendUp && node->texIdRoof[0] != TEX_NONE) || (node->extendDown && node->texIdFloor[0] != TEX_NONE);
						if (wallExists)
						{
							rayHit = side ? RAY_HIT_SIDE : RAY_HIT_FRONT;
							rayEncounters++;
						}
					}
				}

				// ====================================
				// RENDER
				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				if (rayHit)
				{
					Vector2f intersection{ rayStart + rayDir * distance };
					float depth{ Projection(intersection - m_frame.viewPos, m_frame.viewForward * m_rayConfig.farClip).Length() };
					float renderDepth = depth / m_mapMaxDepth;
					const int halfHeight{ static_cast<int>((1 + (m_rayConfig.yResolution / 2) / depth) * m_frame.fovWallMultiplier) };
					const int fullHeight{ halfHeight * 2};

					int mid{ static_cast<int>(m_frame.viewPitch + (m_rayConfig.yResolution / 2)) };
					int bottom{ mid - halfHeight };
					int top{ mid + halfHeight };

					// Draw textured vertical line segments for wall
					bool bWallFinished = false;
					int renderingUp = 0;
					int renderingDown = 0;

					GridNode& node = m_map->pNodes[wallNodeIndex];
					const SDL_Surface* pWallTexture{ nullptr };
					
					int texX = -1;
					auto CalcTexX = [&]()
					{
						// X Index into WallTexture = x position inside cell
						texX = static_cast<int>((rayHit == RAY_HIT_FRONT ? (intersection.x - mapCheck.x) : (intersection.y - mapCheck.y)) * (pWallTexture->w - 1));
						if (texX < 0)
						{
							texX += pWallTexture->w;
						}
						ASSERT(texX < pWallTexture->w && texX >= 0);
					};
					if (node.texIdWall != TEX_NONE)
					{
						pWallTexture = pMapTextures->GetSDLSurface(node.texIdWall);
						CalcTexX();
					}

					while (!bWallFinished)
					{
						// Loop to draw various wall strips. First used to draw 'core' wall strip. Then, upper wall strips, followed by lower wall strips.
						for (int screenY = std::max(0, bottom); screenY <= top; screenY++)
						{
							// Respect any drawFlags specified in editor 
							if (node.drawFlags != eDrawFlags::DRAW_DEFAULT)
							{
								if (rayHit == RAY_HIT_SIDE)
								{
									if (rayDir.x > 0 && !(node.drawFlags & DRAW_W))
									{
										break;
									}
									if (rayDir.x < 0 && !(node.drawFlags & DRAW_E))
									{
										break;
									}
								}
								else
								{
									if (rayDir.y > 0 && !(node.drawFlags & DRAW_N))
									{
										break;
									}
									if (rayDir.y < 0 && !(node.drawFlags & DRAW_S))
									{
										break;
									}
								}
							}

							// Walls can be drawn based on Floor/Wall/Roof. As such, certain visits to this loop may have no texture active and should be skipped.
							if (!pWallTexture)
							{
								break;
							}

							// If we go off the bottom of the screen, skip any remaining wall strip.
							if (screenY >= m_rayConfig.yResolution)
							{
								break;
							}

							// Draw only if our depth is nearer than any existing pixel
							const int screenIndex{ screenX + (screenY * m_rayConfig.xResolution) };
							if (renderDepth < m_bgTexDepth[screenIndex])
							{
								// Y Index into WallTexture = percentage through current Y forloop
								int texY = (pWallTexture->h - 1) - static_cast<int>((static_cast<float>(screenY - bottom) / (top - bottom)) * (pWallTexture->h - 1));
								ASSERT(texY < pWallTexture->h && texY >= 0);

								Uint32* pixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pWallTexture->pixels) + (texY * pWallTexture->pitch) + (texX * pWallTexture->format->BytesPerPixel));
								Uint8 r, g, b, a;
								SDL_GetRGBA(*pixel, pWallTexture->format, &r, &g, &b, &a);
								if (!a)
								{
									continue;
								}

								GLuint& colByte = m_bgTexRGBA[screenIndex];
								colByte = 0;
								colByte |= (r << 0);
								colByte |= (g << 8);
								colByte |= (b << 16);
								colByte |= (a << 24);

								m_bgTexDepth[screenIndex] = renderDepth;
							}
						}

						// Lambda function used to cleanup picture. Corrects small pixel gaps formed by Roof/Floor extending into walls above/below the playspace.
						auto FixSeams = [&](int yStart, int yStep, const GLuint& correctivePixel)
						{
							yStart += yStep;
							constexpr int seamCorrectionLimit{ 10 };
							const int seamCorrectionEnd = yStart + (seamCorrectionLimit * yStep);

							// Detect whether this strip is a seam or just the end of a wall
							bool bIsSeam{ false };
							for (int screenY = yStart; std::abs(screenY - seamCorrectionEnd) > 0; screenY += yStep)
							{
								if (screenY >= m_rayConfig.yResolution || screenY < 0)
								{
									continue;
								}

								const int screenIndex{ screenX + (screenY * m_rayConfig.xResolution) };
								GLuint& nextColByte = m_bgTexRGBA[screenIndex];
								bool nextByteIsInDepthRange = abs(m_bgTexDepth[screenIndex] - renderDepth) < m_rayConfig.correctivePixelDepthTolerance;
								if (nextColByte && nextByteIsInDepthRange)
								{
									bIsSeam = true;
									break;
								}
							}
							if (!bIsSeam)
							{
								return;
							}

							// If this is a seam (ie. wall connected to floor/ceiling) fill pixels sequentially until first non-empty pixel is reached
							for (int screenY = yStart; std::abs(screenY - seamCorrectionEnd) > 0; screenY += yStep)
							{
								if (screenY >= m_rayConfig.yResolution || screenY < 0)
								{
									continue;
								}

								const int screenIndex{ screenX + (screenY * m_rayConfig.xResolution) };
								GLuint& nextColByte = m_bgTexRGBA[screenIndex];
								bool nextByteIsInDepthRange = abs(m_bgTexDepth[screenIndex] - renderDepth) < m_rayConfig.correctivePixelDepthTolerance;
								if (nextColByte == 0 || !nextByteIsInDepthRange)
								{
									if (m_rayConfig.highlightCorrectivePixels)
									{
										nextColByte |= (255 << 0);
										nextColByte |= (0 << 8);
										nextColByte |= (0 << 16);
										nextColByte |= (255 << 24);
									}
									else
									{
										nextColByte = correctivePixel;
									}
									m_bgTexDepth[screenIndex] = renderDepth;
								}
								else
								{
									break;
								}
							}
						};

						// Prepare data for next iteration to render upper wall strips.
						if (node.extendUp > renderingUp++)
						{
							bottom = top + 1;
							top = bottom + fullHeight;
							
							if (node.texIdRoof[0] != TEX_NONE)
							{
								pWallTexture = pMapTextures->GetSDLSurface(node.texIdRoof[0]);
							}
							else if (node.texIdWall != TEX_NONE)
							{
								pWallTexture = pMapTextures->GetSDLSurface(node.texIdWall);
							}
							else
							{
								pWallTexture = nullptr;
							}

							if (texX == -1 && pWallTexture)
							{
								CalcTexX();
							}

							if (pWallTexture && renderingUp == 1 && node.texIdWall == TEX_NONE)
							{
								const Uint32* correctivePixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pWallTexture->pixels) + ((pWallTexture->w - 1) * pWallTexture->pitch) + (texX * pWallTexture->format->BytesPerPixel));
								Uint8 r, g, b, a;
								SDL_GetRGBA(*correctivePixel, pWallTexture->format, &r, &g, &b, &a);
								FixSeams(bottom, -1, r | (g << 8) | (b << 16) | (a << 24));
							}
						}
						// Prepare data for next iteration to render lower wall strips.
						else if (node.extendDown > renderingDown++)
						{
							if (renderingDown == 1)
							{
								bottom = mid - halfHeight;
								top = mid + halfHeight;
							}
							top = bottom - 1;
							bottom = top - fullHeight;
							
							if (node.texIdFloor[0] != TEX_NONE)
							{
								pWallTexture = pMapTextures->GetSDLSurface(node.texIdFloor[0]);
							}
							else if (node.texIdWall != TEX_NONE)
							{
								pWallTexture = pMapTextures->GetSDLSurface(node.texIdWall);
							}
							else
							{
								pWallTexture = nullptr;
							}

							if (texX == -1 && pWallTexture)
							{
								CalcTexX();
							}

							if (pWallTexture && renderingDown == 1 && node.texIdWall == TEX_NONE)
							{
								const Uint32* correctivePixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pWallTexture->pixels) + (texX * pWallTexture->format->BytesPerPixel));
								Uint8 r, g, b, a;
								SDL_GetRGBA(*correctivePixel, pWallTexture->format, &r, &g, &b, &a);
								FixSeams(top, 1, r | (g << 8) | (b << 16) | (a << 24));
							}
						}
						else
						{
							bWallFinished = true;
						}
					}
				}
			}
		}
		return 0;
	};
	START_PROFILE("Raycast Walls")
	Spear::TaskHandle RaycastWallsTaskHandle;
	threader.DispatchTaskDistributed(RaycastWallsTask, &RaycastWallsTaskHandle, m_rayConfig.threads);
	RaycastWallsTaskHandle.WaitForTaskComplete();
	END_PROFILE("Raycast Walls")

	// Upload Raycast image for this frame
	renderer.SetBackgroundTextureDataRGBA(m_bgTexRGBA, m_bgTexDepth, m_rayConfig.xResolution, m_rayConfig.yResolution);
	ClearBackgroundArrays();
}
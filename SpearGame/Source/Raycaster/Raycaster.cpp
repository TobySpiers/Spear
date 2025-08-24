#include "Core/Core.h"
#include "Core/ServiceLocator.h"
#include "Core/ThreadManager.h"
#include "Core/FrameProfiler.h"
#include "Graphics/ScreenRenderer.h"
#include "Graphics/TextureArray.h"
#include "Graphics/ShaderCompiler.h"

#include "Raycaster.h"
#include "RaycasterConfig.h"
#include "LevelFileManager.h"
#include "GlobalTextureBatches.h"
#include <algorithm>

PanelRaycaster Raycaster::debugPanel;

GLuint* Raycaster::m_bgTexRGBA{nullptr};
GLfloat* Raycaster::m_bgTexDepth{nullptr};
MapData* Raycaster::m_map{ nullptr };
RaycasterConfig Raycaster::m_rayConfig;
RaycastSprite Raycaster::m_sprites[Raycaster::RAYCAST_SPRITE_LIMIT];
int Raycaster::m_spriteCount{0};
Raycaster::RaycastFrameData Raycaster::m_frame;
Raycaster::RaycastSpriteData Raycaster::m_frameSprites[RAYCAST_SPRITE_LIMIT];
int Raycaster::m_numSpritesToRender{0};
Raycaster::RaycastComputeShader Raycaster::m_computeShader;
bool Raycaster::m_bSoftwareRendering{false};
int Raycaster::m_softwareRenderingThreads{30};

constexpr float FOV_MIN{ 35.f };
constexpr float FOV_MAX{ 120.f };

void Raycaster::RecreateBackgroundArrays(int width, int height)
{
	delete[] m_bgTexRGBA;
	m_bgTexRGBA = new GLuint[width * height];
	std::fill(m_bgTexRGBA, m_bgTexRGBA + (width * height), GLuint(0));

	delete[] m_bgTexDepth;
	m_bgTexDepth = new GLfloat[width * height];
	std::fill(m_bgTexDepth, m_bgTexDepth + (width * height), GLfloat(m_rayConfig.farClip));
}

void Raycaster::ClearRaycasterArrays()
{
	std::fill(m_bgTexRGBA, m_bgTexRGBA + (m_rayConfig.xResolution * m_rayConfig.yResolution), GLuint(0));
	std::fill(m_bgTexDepth, m_bgTexDepth + (m_rayConfig.xResolution * m_rayConfig.yResolution), GLfloat(m_rayConfig.farClip));
}

int Raycaster::XResolutionPerThread()
{
	return m_rayConfig.xResolution / m_softwareRenderingThreads;
}

int Raycaster::YResolutionPerThread()
{
	return m_rayConfig.yResolution / m_softwareRenderingThreads;
}

void Raycaster::Init(MapData& map)
{
	if (m_bgTexRGBA == nullptr)
	{
		RecreateBackgroundArrays(m_rayConfig.xResolution, m_rayConfig.yResolution);
	}
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

	// FoV naturally affects floor/ceiling height but not wall height... These LUTs approximate sensible wall height and sprite widths
	// There is probably a simple formula to express these as a curve but the current approach works well enough
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
		{120.f, 0.63f}
	};
	constexpr int fovSpriteLUTSize{18};
	const Vector2f fovSpriteLUT[fovSpriteLUTSize] = {
		{35.f, 1.59f},
		{40.f, 1.37f},
		{45.f, 1.21f},
		{50.f, 1.07f},
		{55.f, 0.96f},
		{60.f, 0.87f},
		{65.f, 0.79f},
		{70.f, 0.72f},
		{75.f, 0.65f},
		{80.f, 0.6f},
		{85.f, 0.55f},
		{90.f, 0.5f},
		{95.f, 0.46f},
		{100.f, 0.42f},
		{105.f, 0.385f},
		{110.f, 0.35f},
		{115.f, 0.32f},
		{120.f, 0.29f}
	};

	// Update cached data
	m_frame.fovWallMultiplier = Lerp(fovResult, fovWallHeightLUT, fovWallHeightLUTSize);
	m_frame.fovSpriteMultiplier = Lerp(fovResult, fovSpriteLUT, fovSpriteLUTSize);
	m_frame.fov = TO_RADIANS(fovResult);
}

Vector2i Raycaster::GetResolution()
{
	return Vector2i(m_rayConfig.xResolution, m_rayConfig.yResolution);
}

MapData* Raycaster::GetMap()
{
	return m_map;
}

RaycastSprite& Raycaster::MakeSprite()
{
	ASSERT(m_spriteCount < RAYCAST_SPRITE_LIMIT);
	return m_sprites[m_spriteCount++];
}

void Raycaster::ClearSprites()
{
	m_spriteCount = 0;
}

void Raycaster::Draw2DGrid(const Vector2f& pos, const float angle)
{
	START_PROFILE("2D Raycasting");
	Spear::Renderer& rend = Spear::ServiceLocator::GetScreenRenderer();
	ClearRaycasterArrays();
	rend.SetBackgroundTextureDataRGBA(m_bgTexRGBA, m_bgTexDepth, m_rayConfig.xResolution, m_rayConfig.yResolution);

	constexpr float depthWorld = 0.5f;
	constexpr float depthPlayer = 0.25f;

	// Draw player
	Spear::Renderer::LinePolyData playerPoly;
	playerPoly.colour = Colour4f::Red();
	playerPoly.radius = 0.2f * m_rayConfig.scale2D;
	playerPoly.segments = 3;
	playerPoly.pos = Spear::Core::GetWindowSize().ToFloat() / 2;
	playerPoly.rotation = angle;
	playerPoly.depth = depthPlayer;
	rend.AddLinePoly(playerPoly);

	const Vector2f camOffset = (Spear::Core::GetWindowSize().ToFloat() / 2) - pos * m_rayConfig.scale2D;

	// Draw tiles
	constexpr float tiledepthOpacities[3] = {1.f, 0.7f, 0.5f};
	for (int x = 0; x < m_map->gridWidth; x++)
	{
		for (int y = 0; y < m_map->gridHeight; y++)
		{
			const int nodeIndex = x + (y * m_map->gridWidth);
			GridNode& node = m_map->pNodes[nodeIndex];

			int texId = node.texIdWall;
			int tileDepth = 0;
			if (texId == TEX_NONE)
			{
				for (int i = 0; i < 2; i++)
				{
					texId = node.texIdFloor[i];
					tileDepth = i + 1;
					if (texId != TEX_NONE)
					{
						break;
					}
				}
			}

			if (texId != TEX_NONE)
			{
				Spear::Renderer::SpriteData sprite;
				sprite.pos = Vector2f(x, y) * m_rayConfig.scale2D;
				sprite.pos += (Vector2f(m_rayConfig.scale2D, m_rayConfig.scale2D) / 2) + camOffset;
				sprite.size = 1.2f;
				sprite.texLayer = texId;
				sprite.opacity = tiledepthOpacities[tileDepth];
				sprite.depth = depthWorld;

				rend.AddSprite(sprite, GlobalTextureBatches::BATCH_TILESET_1);
			}
		}
	}

	// Draw sprites
	const float spriteRawSize = 64.f / Spear::Renderer::Get().GetBatchTextures(GlobalTextureBatches::BATCH_SPRITESET_1)->GetWidth(); // assumes textures are square
	for (int i = 0; i < m_spriteCount; i++)
	{
		RaycastSprite& spriteData = m_sprites[i];
		Spear::Renderer::SpriteData sprite;
		sprite.pos = (spriteData.spritePos + Vector2f(-0.5f)) * m_rayConfig.scale2D;
		sprite.pos += (Vector2f(m_rayConfig.scale2D, m_rayConfig.scale2D) / 2) + camOffset;
		sprite.size = spriteRawSize * spriteData.size;
		sprite.size.y /= 2;
		sprite.texLayer = spriteData.textureId;

		rend.AddSprite(sprite, GlobalTextureBatches::BATCH_SPRITESET_1);
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
	int maxRays{ 500 };
	for(int x = 0; x < maxRays; x++)
	{
		Vector2f rayStart = pos;
		Vector2f rayEnd{ fovLeftExtent - (raySpacingDir * raySpacing * (x * m_rayConfig.xResolution / maxRays)) };
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
		Spear::Renderer::LineData line;
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

	END_PROFILE("2D Raycasting");
}

void Raycaster::Draw3DGrid(const Vector2f& inPos, float inPitch, const float angle)
{
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
		m_frame.screenPlaneVector = Normalize(m_frame.screenPlaneVector);

		// unit vectors representing directions to form left/right edge of FoV
		m_frame.fovMinAngle = Vector2f(cos(angle - halfFov), sin(angle - halfFov));
		m_frame.fovMaxAngle = Vector2f(cos(angle + halfFov), sin(angle + halfFov));		

		m_frame.planeHeights.x = m_map->planeHeights[PLANE_HEIGHT_INNER];
		m_frame.planeHeights.y = m_map->planeHeights[PLANE_HEIGHT_OUTER];
	}

	// Format sprite data into screen coordinates
	PreProcessSprites();

	if (!m_bSoftwareRendering)
	{
		Draw3DGridCompute(inPos, inPitch, angle);
	}
	else
	{
		Draw3DGridCPU(inPos, inPitch, angle);
	}
}

void Raycaster::PreProcessSprites()
{
	m_numSpritesToRender = 0;
	for (int i = 0; i < m_spriteCount; i++)
	{
		const RaycastSprite& sprite = m_sprites[i];
		const Vector2f relativePosition = sprite.spritePos - m_frame.viewPos;

		// NOTE: m_frame.viewForward is a unit vector
		// Dot product of relativePosition with viewForward gives us the forward depth to the sprite
		const float forwardDistance = Dot(relativePosition, m_frame.viewForward);
		if (forwardDistance < 0.f)
		{
			continue;
		}

		// NOTE: m_frame.screenPlaneVector is a unit vector
		// Dot product of relativePosition with screenPlaneVector gives us the horizonal distance to the sprite from viewForward (midscreen)
		const float rightDistance = Dot(relativePosition, m_frame.screenPlaneVector);

		// SohCahToa (Toa) also means: tan(angle) * adjacentLength = oppositeLength
		// in this case, 'angle' is half our fov, and 'adjacent' is our forward distance = this results in the horizontal distance from viewForward to screen's edge
		const float halfViewWidth = tanf(m_frame.fov / 2.f) * forwardDistance;

		// Convert halfViewWidth and rightDistance into a range between -1 and +1, then convert that into a percentage from 0.f to 1.f
		float screenPercent = (rightDistance / halfViewWidth);
		screenPercent = (screenPercent * 0.5f) + 0.5f;

		// ScreenPos.X is simply screenPercent multiplied by xResolution
		// ScreenPos.Y is middle of screen (yResolution / 2), shifted by viewPitch (vertical look), with spriteHeight (scaled by distance) applied
		const Vector2i screenPos{ int(m_rayConfig.xResolution * screenPercent), ((m_rayConfig.yResolution / 2) + int(m_frame.viewPitch) + int(sprite.height / forwardDistance)) };

		// Calculate sprite size scaled by distance/resolution/fov
		Vector2f spriteSize = ((sprite.size / forwardDistance) * Vector2f(m_rayConfig.xResolution * m_frame.fovSpriteMultiplier, m_rayConfig.yResolution * (m_frame.fovWallMultiplier / 2)));

		// Calculate edges based on size
		Vector2i screenStart = screenPos - (spriteSize / 2).ToInt();
		Vector2i screenEnd = screenPos + (spriteSize / 2).ToInt();

		if (screenEnd.x < 0 || screenStart.x >= m_rayConfig.xResolution)
		{
			continue;
		}

		const float renderDepth = forwardDistance / m_rayConfig.farClip;

		const int spId = m_numSpritesToRender++;
		m_frameSprites[spId].spriteStart = screenStart;
		m_frameSprites[spId].spriteEnd = screenEnd;
		m_frameSprites[spId].spriteTex = sprite.textureId;
		m_frameSprites[spId].spriteDepth = renderDepth;
	}

	// Sort sprites so furthest are rendered first - useful if we want to support transparency
	std::sort(m_frameSprites, m_frameSprites + m_numSpritesToRender, [](const RaycastSpriteData& a, const RaycastSpriteData& b) {
		return a.spriteDepth > b.spriteDepth;
	});
}

void Raycaster::Draw3DGridCPU(const Vector2f& inPos, float inPitch, const float angle)
{
	Spear::Renderer& renderer = Spear::ServiceLocator::GetScreenRenderer();
	Spear::ThreadManager& threader = Spear::ServiceLocator::GetThreadManager();

	// Floor/Ceiling Casting
	auto RaycastPlanesTask = [](int taskId)
	{
		const Spear::TextureBase* pMapTextures = Spear::ServiceLocator::GetScreenRenderer().GetBatchTextures(0);

		int yLowerBound = YResolutionPerThread() * taskId;
		int yUpperBound = taskId == m_softwareRenderingThreads - 1 ? m_rayConfig.yResolution : yLowerBound + YResolutionPerThread(); // avoid skipping pixels under non-perfect division

		for (int y = yLowerBound; y < yUpperBound; y++) // for the chunk of Y pixels allocated to this thread...
		{
			bool bIsFloor = y < (static_cast<float>(m_rayConfig.yResolution) / 2) + m_frame.viewPitch;

			// Current y position compared to the center of the screen (the horizon)
			int rayPitch;
			if (bIsFloor)
			{
				rayPitch = ((m_rayConfig.yResolution - y - 1) - (m_rayConfig.yResolution / 2)) + m_frame.viewPitch;
			}
			else
			{
				rayPitch = (y - (m_rayConfig.yResolution / 2)) - m_frame.viewPitch;
			}

			// Forward distance from the camera to hit the floor for the current row.
			// 0.5 is the z position exactly in the middle between floor and ceiling.
			float rowDistance[2];
			const float defaultDistance = m_frame.viewHeight / rayPitch;
			rowDistance[0] = defaultDistance * m_frame.planeHeights.x;
			rowDistance[1] = defaultDistance * m_frame.planeHeights.y;

			// Vector representing position offset equivalent to 1 pixel right (imagine topdown 2D view, this 'jumps' horizontally by 1 ray)
			Vector2f rayStep[2];
			const Vector2f rayPixelWidth = (m_frame.fovMaxAngle - m_frame.fovMinAngle) / m_rayConfig.xResolution;
			rayStep[0] = rowDistance[0] * rayPixelWidth;
			rayStep[1] = rowDistance[1] * rayPixelWidth;

			// endpoint of first ray in row (left)
			// essentially the 'left-most ray' along a length (depth) of rowDistance
			Vector2f rayEnd[2];
			rayEnd[0] = m_frame.viewPos + rowDistance[0] * m_frame.fovMinAngle;
			rayEnd[1] = m_frame.viewPos + rowDistance[1] * m_frame.fovMinAngle;

			// calculate 'depth' for this strip of floor (same depth will be shared for all drawn pixels in one row of X)
			Vector2f rayStart[2];
			float rayDepth[2];
			rayStart[0] = m_frame.viewPos + Projection(rowDistance[0] * m_frame.fovMinAngle, m_frame.viewForward.Normal());
			rayStart[1] = m_frame.viewPos + Projection(rowDistance[1] * m_frame.fovMinAngle, m_frame.viewForward.Normal());
			rayDepth[0] = (rayEnd[0] - rayStart[0]).Length() / m_rayConfig.farClip;
			rayDepth[1] = (rayEnd[1] - rayStart[1]).Length() / m_rayConfig.farClip;

			// Indexes into texture/depth arrays for the start of the row equal to Y value
			const int rowIndex = y * m_rayConfig.xResolution;

			// Draw background texture
			for (int x = 0; x < m_rayConfig.xResolution; ++x)
			{
				// Prevent drawing floor textures behind camera
				if (rowDistance[0] > 0)
				{
					for (int layer = 0; layer < 2; layer++)
					{
						// Render floor
						const int mapCellX = (int)(rayEnd[layer].x);
						const int mapCellY = (int)(rayEnd[layer].y);

						if (const GridNode* node = m_map->GetNode(mapCellX, mapCellY))
						{
							// Floor tex sampling
							if ((bIsFloor && node->texIdFloor[layer] != eLevelTextures::TEX_NONE) || (!bIsFloor && node->texIdRoof[layer] != eLevelTextures::TEX_NONE))
							{
								const SDL_Surface* pWorldTexture = pMapTextures->GetSDLSurface(bIsFloor ? node->texIdFloor[layer] : node->texIdRoof[layer]);
								ASSERT(pWorldTexture);

								int texX = static_cast<int>((rayEnd[layer].x - mapCellX) * pWorldTexture->w);
								int texY = static_cast<int>((rayEnd[layer].y - mapCellY) * pWorldTexture->h);
								if (texX < 0)
									texX += pWorldTexture->w;
								if (texY < 0)
									texY += pWorldTexture->h;

								ASSERT(texX < pWorldTexture->w);
								ASSERT(texY < pWorldTexture->h);
								Uint32* pixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pWorldTexture->pixels) + (texY * pWorldTexture->pitch) + (texX * pWorldTexture->format->BytesPerPixel));
								Uint8 r, g, b, a;
								SDL_GetRGBA(*pixel, pWorldTexture->format, &r, &g, &b, &a);

								const int textureArrayIndex{ rowIndex + x };
								GLuint& colByte = m_bgTexRGBA[textureArrayIndex];
								colByte |= (r << 0);
								colByte |= (g << 8);
								colByte |= (b << 16);
								colByte |= (a << 24);

								m_bgTexDepth[textureArrayIndex] = rayDepth[layer];

								// We're drawing the floors nearest-first, so break this inner for-loop as soon as we draw a pixel (ie. no need to calculate pixels BEHIND this)
								break;
							}
						}
					}
					rayEnd[0] += rayStep[0];
					rayEnd[1] += rayStep[1];
				}
			}
		}

		return 0;
	};
	START_PROFILE("Raycast Planes")
	Spear::TaskHandle RaycastPlanesTaskHandle;
	threader.DispatchTaskDistributed(RaycastPlanesTask, &RaycastPlanesTaskHandle, m_softwareRenderingThreads);
	RaycastPlanesTaskHandle.WaitForTaskComplete();
	END_PROFILE("Raycast Planes")

	// Using DDA (digital differential analysis) to quickly calculate intersections
	auto RaycastWallsTask = [](int taskId)
	{
		const Spear::TextureBase* pMapTextures = Spear::ServiceLocator::GetScreenRenderer().GetBatchTextures(0);

		int xLowerBound = XResolutionPerThread() * taskId;
		int xUpperBound = taskId == m_softwareRenderingThreads - 1 ? m_rayConfig.xResolution : xLowerBound + XResolutionPerThread(); // avoid skipping pixels under non-perfect division

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
					float renderDepth = depth / m_rayConfig.farClip;
					const float halfHeight{ (1 + (m_rayConfig.yResolution / 2) / depth) * m_frame.fovWallMultiplier };
					const float fullHeight{ halfHeight * 2};

					int mid{ static_cast<int>(m_frame.viewPitch + (m_rayConfig.yResolution / 2)) };
					float bottom{ mid - halfHeight };
					float top{ mid + halfHeight };

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
						for (int screenY = std::max(0, static_cast<int>(bottom)); screenY <= static_cast<int>(top); screenY++)
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
								int texY = (pWallTexture->h - 1) - static_cast<int>((static_cast<float>(screenY - static_cast<int>(bottom)) / (static_cast<int>(top) - static_cast<int>(bottom))) * (pWallTexture->h - 1));
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
	threader.DispatchTaskDistributed(RaycastWallsTask, &RaycastWallsTaskHandle, m_softwareRenderingThreads);
	RaycastWallsTaskHandle.WaitForTaskComplete();
	END_PROFILE("Raycast Walls")

	auto RaycastSpritesTask = [](int taskId)
	{
		const Vector2i cutoffMin(0, YResolutionPerThread() * taskId);
		const Vector2i cutoffMax(m_rayConfig.xResolution, YResolutionPerThread() * (taskId + 1));

		const Spear::TextureBase* pSpriteTextures = Spear::ServiceLocator::GetScreenRenderer().GetBatchTextures(GlobalTextureBatches::BATCH_SPRITESET_1);

		for (int i = 0; i < m_numSpritesToRender; i++)
		{
			const SDL_Surface* pSpriteTexture = pSpriteTextures->GetSDLSurface(m_frameSprites[i].spriteTex);

			for (int y = std::max(cutoffMin.y, m_frameSprites[i].spriteStart.y); y <= std::min(m_frameSprites[i].spriteEnd.y, cutoffMax.y - 1); y++)
			{
				for (int x = std::max(cutoffMin.x, m_frameSprites[i].spriteStart.x); x <= std::min(m_frameSprites[i].spriteEnd.x, cutoffMax.x - 1); x++)
				{
					int screenIndex = x + (y * m_rayConfig.xResolution);
					if (m_frameSprites[i].spriteDepth < m_bgTexDepth[screenIndex])
					{
						int texX = (float(x - m_frameSprites[i].spriteStart.x) / (m_frameSprites[i].spriteEnd.x - m_frameSprites[i].spriteStart.x)) * pSpriteTexture->w;
						int texY = pSpriteTexture->h - (float(y - m_frameSprites[i].spriteStart.y) / (m_frameSprites[i].spriteEnd.y - m_frameSprites[i].spriteStart.y)) * (pSpriteTexture->h - 1);

						Uint32* pixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pSpriteTexture->pixels) + (texY * pSpriteTexture->pitch) + (texX * pSpriteTexture->format->BytesPerPixel));
						Uint8 r, g, b, a;
						SDL_GetRGBA(*pixel, pSpriteTexture->format, &r, &g, &b, &a);
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

						m_bgTexDepth[screenIndex] = m_frameSprites[i].spriteDepth;
					}
				}
			}
		}

		return 0;
	};
	START_PROFILE("Raycast Sprites")
	Spear::TaskHandle RaycastSpritesTaskHandle;
	threader.DispatchTaskDistributed(RaycastSpritesTask, &RaycastSpritesTaskHandle, m_softwareRenderingThreads);
	RaycastSpritesTaskHandle.WaitForTaskComplete();
	END_PROFILE("Raycast Sprites")

	START_PROFILE("Raycast Upload")
	// Upload Raycast image for this frame
	renderer.SetBackgroundTextureDataRGBA(m_bgTexRGBA, m_bgTexDepth, m_rayConfig.xResolution, m_rayConfig.yResolution);
	ClearRaycasterArrays();
	END_PROFILE("Raycast Upload")
}

void Raycaster::Draw3DGridCompute(const Vector2f& pos, float pitch, const float angle)
{
	Spear::Renderer& renderer = Spear::ServiceLocator::GetScreenRenderer();

	START_PROFILE("Compute Shader")

	if(!m_computeShader.isInitialised)
	{
		m_computeShader.isInitialised = true;

		// Compile shaders
		m_computeShader.program[0] = Spear::ShaderCompiler::CreateShaderProgram("../Shaders/RaycastPlanesCS.glsl");
		m_computeShader.program[1] = Spear::ShaderCompiler::CreateShaderProgram("../Shaders/RaycastWallsCS.glsl");

		// GridNodes Binding SSBO (Shader Storage Buffer Object) - Used to pass array data to shader
		glGenBuffers(1, &m_computeShader.gridnodesSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_computeShader.gridnodesSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, m_map->TotalNodes() * sizeof(GridNode), m_map->pNodes, GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, m_computeShader.gridnodesSSBO); // bind slot 2

		// Sprites Binding SSBO
		glGenBuffers(1, &m_computeShader.spritesSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_computeShader.spritesSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, RAYCAST_SPRITE_LIMIT * sizeof(RaycastSpriteData), m_frameSprites, GL_STATIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, m_computeShader.spritesSSBO); // bind slot 5

		// RayConfig Binding UBO (Uniform Buffer Object) - Used to pass a single struct as a uniform to shader
		glGenBuffers(1, &m_computeShader.rayconfigUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, m_computeShader.rayconfigUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(RaycasterConfig), &m_rayConfig, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 3, m_computeShader.rayconfigUBO); // bind slot 3
		for (int i = 0; i < m_computeShader.programSize; i++)
		{
			GLuint configBlockIndex = glGetUniformBlockIndex(m_computeShader.program[i], "raycasterConfig");
			glUniformBlockBinding(m_computeShader.program[i], configBlockIndex, 3);  // binding point 3
		}

		// FrameData Binding UBO
		glGenBuffers(1, &m_computeShader.framedataUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, m_computeShader.framedataUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(RaycastFrameData), &m_frame, GL_STATIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, 4, m_computeShader.framedataUBO); // bind slot 4
		for (int i = 0; i < m_computeShader.programSize; i++)
		{
			GLuint framedataBlockIndex = glGetUniformBlockIndex(m_computeShader.program[i], "frameData");
			glUniformBlockBinding(m_computeShader.program[i], framedataBlockIndex, 4);  // binding point 4
		}

		// Cache uniform locations for shader programs
		for (int i = 0; i < m_computeShader.programSize; i++)
		{
			m_computeShader.gridDimensionsLoc[i] = glGetUniformLocation(m_computeShader.program[i], "gridDimensions");
			m_computeShader.worldTexturesLoc[i] = glGetUniformLocation(m_computeShader.program[i], "worldTextures");
			m_computeShader.spriteTexturesLoc[i] = glGetUniformLocation(m_computeShader.program[i], "spriteTextures");
			m_computeShader.spriteCountLoc[i] = glGetUniformLocation(m_computeShader.program[i], "spriteCount");
		}
	}
	else
	{
		// If buffers already exist, just update the data

		// Upload GridNode data
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_computeShader.gridnodesSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_map->TotalNodes() * sizeof(GridNode), m_map->pNodes);

		// Upload Sprites data
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_computeShader.spritesSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, m_numSpritesToRender * sizeof(RaycastSpriteData), m_frameSprites);

		// Upload RayConfig
		glBindBuffer(GL_UNIFORM_BUFFER, m_computeShader.rayconfigUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(RaycasterConfig), &m_rayConfig);

		// Upload FrameData
		glBindBuffer(GL_UNIFORM_BUFFER, m_computeShader.framedataUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(RaycastFrameData), &m_frame);

	}

	// Format & Bind screen texture
	Spear::Texture& screenTexture = renderer.GetBackgroundTextureForNextFrame();
	if(screenTexture.GetWidth() != m_rayConfig.xResolution || screenTexture.GetHeight() != m_rayConfig.yResolution)
	{
		screenTexture.Resize(m_rayConfig.xResolution, m_rayConfig.yResolution);
	}
	glBindImageTexture(0, screenTexture.GetGpuTextureId(), 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA8);

	// Format & Bind depth buffer
	GLuint depthTexture = renderer.GetBackgroundDepthBufferForNextFrame();
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_R32F,
		m_rayConfig.xResolution,
		m_rayConfig.yResolution,
		0,
		GL_RED,
		GL_FLOAT,
		NULL
	);
	glBindImageTexture(1, depthTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32F);

	// Pass in TextureArray for world textures
	const Spear::TextureBase* worldTextures = renderer.GetBatchTextures(GlobalTextureBatches::BATCH_TILESET_1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, worldTextures->GetGpuTextureId());

	// Pass in TextureArray for sprites
	const Spear::TextureBase* spriteTextures = renderer.GetBatchTextures(GlobalTextureBatches::BATCH_SPRITESET_1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D_ARRAY, spriteTextures->GetGpuTextureId());

	// Run shader programs
	for (int i = 0; i < m_computeShader.programSize; i++)
	{
		glUseProgram(m_computeShader.program[i]);
		glUniform2i(m_computeShader.gridDimensionsLoc[i], m_map->gridWidth, m_map->gridHeight);
		glUniform1i(m_computeShader.spriteCountLoc[i], m_numSpritesToRender);
		glUniform1i(m_computeShader.worldTexturesLoc[i], 0); // set sampler to read world textures from GL_TEXTURE0
		glUniform1i(m_computeShader.spriteTexturesLoc[i], 1); // set sampler to read sprite textures from GL_TEXTURE1

		switch (i)
		{
			case 0: // PLANES - 1 invocation per screen pixel
				glDispatchCompute(screenTexture.GetWidth(), screenTexture.GetHeight(), 1);
				break;
			case 1: // WALLS - 1 invocation per screen column
				glDispatchCompute(screenTexture.GetWidth(), 1, 1);
				break;
		}
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	}

	// Unbind world textures
	glBindTexture(GL_TEXTURE_2D_ARRAY, NULL);

	END_PROFILE("Compute Shader")
}
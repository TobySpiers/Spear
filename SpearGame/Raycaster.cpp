#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/ScreenRenderer.h"
#include "SpearEngine/ThreadManager.h"
#include "SpearEngine/TextureArray.h"
#include "SpearEngine/FrameProfiler.h"

#include "Raycaster.h"
#include "LevelManager.h"
#include <algorithm>

GLuint* Raycaster::m_bgTexRGBA{nullptr};
GLfloat* Raycaster::m_bgTexDepth{nullptr};
RaycastParams Raycaster::m_rayConfig;
MapData Raycaster::m_map;
GLfloat Raycaster::m_mapMaxDepth{FLT_MAX};
Raycaster::RaycastFrameData Raycaster::m_frame;

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

void Raycaster::LoadLevel(const char* filename)
{
	LevelManager::LoadLevel(filename, m_map);
	if (m_bgTexRGBA == nullptr)
	{
		RecreateBackgroundArrays(m_rayConfig.xResolution, m_rayConfig.yResolution);
	}
	m_mapMaxDepth = sqrt(pow(m_map.gridWidth, 2) + pow(m_map.gridHeight, 2));
}

void Raycaster::ApplyConfig(const RaycastParams& config)
{ 
		
	if (config.xResolution != m_rayConfig.xResolution || config.yResolution != m_rayConfig.yResolution)
	{
		RecreateBackgroundArrays(config.xResolution, config.yResolution);
	}

	m_rayConfig = config; 
};

void Raycaster::Draw2DLooseWalls(const Vector2f& pos, const float angle, RaycastWall* pWalls, int wallCount)
{
	Spear::ScreenRenderer& rend = Spear::ServiceLocator::GetScreenRenderer();

	// Define the 'screen'
	const float halfFov{ m_rayConfig.fieldOfView / 2 };
	const Vector2f forward{ Normalize(Vector2f(cos(angle), sin(angle))) };
	const Vector2f screenPlaneL{ pos + (Vector2f(cos(angle + halfFov), sin(angle + halfFov)) * m_rayConfig.farClip) };
	const Vector2f screenPlaneR{ pos + (Vector2f(cos(angle - halfFov), sin(angle - halfFov)) * m_rayConfig.farClip) };
	const Vector2f screenVector{ screenPlaneR - screenPlaneL };
	// Define the 'rays'
	const Vector2f raySpacingDir{ forward.Normal() * -1 };
	const float raySpacingLength{ screenVector.Length() / m_rayConfig.xResolution };

	// Draw each wall
	for (int i = 0; i < wallCount; i++)
	{
		Spear::ScreenRenderer::LineData line;
		line.start = pWalls[i].origin;
		line.end = pWalls[i].origin + pWalls[i].vec;
		line.colour = pWalls[i].colour;
		rend.AddLine(line);
	}

	// Draw each ray
	for (int i = 0; i < m_rayConfig.xResolution; i++)
	{
		Vector2f rayEndPoint{screenPlaneL + (raySpacingDir * raySpacingLength * i)};
		Vector2f ray{rayEndPoint - pos};

		// Check each wall...
		Vector2f intersect;
		bool foundIntersect{ false };
		for (int w = 0; w < wallCount; w++)
		{
			RaycastWall& wall = pWalls[w];

			// Check for an intersect...
			Vector2f result;
			if (VectorIntersection(pos, ray, wall.origin, wall.vec, result))
			{
				// Check if it was nearer than any previous discovered intersect...
				float distance{ Vector2f(pos - result).LengthSqr()};
				float existingDistance{ Vector2f(pos - intersect).LengthSqr() };
				if (!foundIntersect || distance < existingDistance)
				{
					// Store result
					intersect = result;
					foundIntersect = true;
				}
			}
		}

		Spear::ScreenRenderer::LineData line;
		line.start = pos;
		line.end = foundIntersect? intersect : rayEndPoint;
		rend.AddLine(line);
	}
}

void Raycaster::Draw3DLooseWalls(const Vector2f& pos, const float angle, RaycastWall* pWalls, int wallCount)
{
	Spear::ScreenRenderer& rend = Spear::ServiceLocator::GetScreenRenderer();
	int lineWidth{ static_cast<int>(Spear::Core::GetWindowSize().x) / m_rayConfig.xResolution };

	// Define a flat 'screen plane' to evenly distribute rays onto
	// This distribution ensures objects do not squash/stretch as they traverse the screen
	// If we don't do this, radial-distribution results in larger gaps between rays at further edges of flat surfaces
	const float halfFov{ m_rayConfig.fieldOfView / 2 };
	const Vector2f forward{ Normalize(Vector2f(cos(angle), sin(angle))) };
	const Vector2f screenPlaneL{ pos + (Vector2f(cos(angle - halfFov), sin(angle - halfFov)) * m_rayConfig.farClip) };
	const Vector2f screenPlaneR{ pos + (Vector2f(cos(angle + halfFov), sin(angle + halfFov)) * m_rayConfig.farClip) };
	const Vector2f screenVector{ screenPlaneR - screenPlaneL };

	// Define ray spacing
	const Vector2f raySpacingDir{ forward.Normal() * -1 };
	const float raySpacingLength{ screenVector.Length() / m_rayConfig.xResolution };

	// For each ray...
	for (int screenX = 0; screenX < m_rayConfig.xResolution; screenX++)
	{
		Vector2f rayEndPoint{ screenPlaneL - (raySpacingDir * raySpacingLength * screenX) };
		Vector2f ray{ rayEndPoint - pos };

		// Initial ray data
		float nearestLength{ m_rayConfig.farClip };
		Colour4f rayColour = Colour4f::Invisible();
		Vector2f intersection{0.f, 0.f};

		// Check each wall...
		for (int w = 0; w < wallCount; w++)
		{
			RaycastWall& wall = pWalls[w];

			// Check for an intersect...
			Vector2f result;
			if (VectorIntersection(pos, ray, wall.origin, wall.vec, result))
			{
				// If it was nearer than previous intersect...
				float newLength{ Vector2f(pos - result).Length() };
				if (newLength < nearestLength)
				{
					// Update ray data
					nearestLength = newLength;
					rayColour = wall.colour;
					intersection = result;
				}
			}
		}

		if (rayColour.a != 0)
		{
			// Project THIS RAY onto the FORWARD VECTOR of the camera to get distance from camera with no fisheye distortion
			float depth{ Projection(intersection - pos, forward * m_rayConfig.farClip).Length() };
			rayColour.a = (1.f / (depth / 4));

			float height{ (Spear::Core::GetWindowSize().y / 2.0f) / depth };
			float mid{ Spear::Core::GetWindowSize().y / 2.0f };

			Spear::ScreenRenderer::LineData line;
			line.start = Vector2f((screenX * lineWidth) + (lineWidth / 2), mid - height);
			line.end = Vector2f((screenX * lineWidth) + (lineWidth / 2), mid + height);
			line.colour = rayColour;
			rend.AddLine(line);
		}
	}
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
	for (int x = 0; x < m_map.gridWidth; x++)
	{
		for (int y = 0; y < m_map.gridHeight; y++)
		{
			const int nodeIndex = x + (y * m_map.gridWidth);
			GridNode& node = m_map.pNodes[nodeIndex];
			bool bWallTexture = node.texIdWall != TEX_NONE;
			bool bRoofTexture = node.texIdRoof != TEX_NONE;
			int texId = bWallTexture ? node.texIdWall : (bRoofTexture ? node.texIdRoof : node.texIdFloor);
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

	const float halfFov{ m_rayConfig.fieldOfView / 2 };
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
			if(mapCheck.x >= 0 && mapCheck.x < m_map.gridWidth && mapCheck.y >= 0 && mapCheck.y < m_map.gridHeight)
			{	
				// if tile is assigned a tex value it EXISTS
				if (m_map.pNodes[mapCheck.x + (mapCheck.y * m_map.gridWidth)].texIdWall != eLevelTextures::TEX_NONE)
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
		m_frame.viewPitch = inPitch * m_rayConfig.yResolution;
		m_frame.viewHeight = 0.625f * m_rayConfig.yResolution;
		m_frame.viewForward = Normalize(Vector2f(cos(angle), sin(angle)));
		m_frame.viewPos = inPos;

		// Screen Plane ray-distribution to avoid squash/stretch warping
		const float halfFov{ m_rayConfig.fieldOfView / 2 };
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
			const int rayPitchFloor = (y - m_rayConfig.yResolution / 2) + m_frame.viewPitch + 2;
			const int rayPitchRoof = (y - m_rayConfig.yResolution / 2) - m_frame.viewPitch + 2;

			// Horizontal distance from the camera to the floor for the current row.
			// 0.5 is the z position exactly in the middle between floor and ceiling.
			const float rowDistanceFloor = m_frame.viewHeight / rayPitchFloor;
			const float rowDistanceRoof = m_frame.viewHeight / rayPitchRoof;

			// vector representing position offset equivalent to 1 pixel right (imagine topdown 2D view, this 'jumps' horizontally by 1 ray)
			const Vector2f floorStep = rowDistanceFloor * (m_frame.fovMaxAngle - m_frame.fovMinAngle) / m_rayConfig.xResolution;
			const Vector2f roofStep = rowDistanceRoof * (m_frame.fovMaxAngle - m_frame.fovMinAngle) / m_rayConfig.xResolution;

			// starting position of first pixel in row (left)
			// essentially the 'left-most ray' along a length (depth) of rowDistance
			Vector2f floorXY = m_frame.viewPos + rowDistanceFloor * m_frame.fovMinAngle;
			Vector2f roofXY = m_frame.viewPos + rowDistanceRoof * m_frame.fovMinAngle;

			// Draw background texture
			for (int x = 0; x < m_rayConfig.xResolution; ++x)
			{
				// Prevent drawing floor textures behind camera
				if (rowDistanceFloor > 0)
				{
					// Render floor
					const int floorCellX = (int)(floorXY.x);
					const int floorCellY = (int)(floorXY.y);

					if (floorCellX >= 0 && floorCellY >= 0 && floorCellX < m_map.gridWidth && floorCellY < m_map.gridHeight)
					{
						// Calculate floor depth 
						float depth{ Projection(floorXY - m_frame.viewPos, m_frame.viewForward * m_rayConfig.farClip).Length() };
						depth /= m_mapMaxDepth;

						// Floor tex sampling
						if (m_map.pNodes[floorCellX + (floorCellY * m_map.gridWidth)].texIdFloor != eLevelTextures::TEX_NONE)
						{
							const SDL_Surface* pFloorTexture = pMapTextures->GetSDLSurface(m_map.pNodes[floorCellX + (floorCellY * m_map.gridWidth)].texIdFloor);
							ASSERT(pFloorTexture);

							int texX = static_cast<int>((floorXY.x - floorCellX) * pFloorTexture->w);
							int texY = static_cast<int>((floorXY.y - floorCellY) * pFloorTexture->h);
							if (texX < 0)
								texX += pFloorTexture->w;
							if (texY < 0)
								texY += pFloorTexture->h;

							ASSERT(texX < pFloorTexture->w);
							ASSERT(texY < pFloorTexture->h);
							Uint32* pixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pFloorTexture->pixels) + (texY * pFloorTexture->pitch) + (texX * pFloorTexture->format->BytesPerPixel));
							Uint8 r, g, b, a;
							SDL_GetRGBA(*pixel, pFloorTexture->format, &r, &g, &b, &a);

							const int textureArrayIndex{ x + ((m_rayConfig.yResolution - (y + 1)) * m_rayConfig.xResolution) };
							GLuint& colByte = m_bgTexRGBA[textureArrayIndex];
							colByte |= (r << 0);
							colByte |= (g << 8);
							colByte |= (b << 16);
							colByte |= (a << 24);

							m_bgTexDepth[textureArrayIndex] = depth;
						}
					}
					floorXY += floorStep;
				}

				// Prevent drawing roof textures behind camera
				if (rowDistanceRoof > 0)
				{
					// Render roof
					const int roofCellX = (int)(roofXY.x);
					const int roofCellY = (int)(roofXY.y);

					if (roofCellX >= 0 && roofCellY >= 0 && roofCellX < m_map.gridWidth && roofCellY < m_map.gridHeight)
					{
						// Calculate roof depth
						float depth{ Projection(roofXY - m_frame.viewPos, m_frame.viewForward * m_rayConfig.farClip).Length() };
						depth /= m_mapMaxDepth;

						// Roof tex sampling
						if (m_map.pNodes[roofCellX + (roofCellY * m_map.gridWidth)].texIdRoof != eLevelTextures::TEX_NONE)
						{
							const SDL_Surface* pRoofTexture = pMapTextures->GetSDLSurface(m_map.pNodes[roofCellX + (roofCellY * m_map.gridWidth)].texIdRoof);
							ASSERT(pRoofTexture);

							int texX = static_cast<int>((roofXY.x - roofCellX) * pRoofTexture->w);
							int texY = static_cast<int>((roofXY.y - roofCellY) * pRoofTexture->h);
							if (texX < 0)
								texX += pRoofTexture->w;
							if (texY < 0)
								texY += pRoofTexture->h;

							ASSERT(texX < pRoofTexture->w);
							ASSERT(texY < pRoofTexture->h);
							Uint32* pixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pRoofTexture->pixels) + (texY * pRoofTexture->pitch) + (texX * pRoofTexture->format->BytesPerPixel));
							Uint8 r, g, b, a;
							SDL_GetRGBA(*pixel, pRoofTexture->format, &r, &g, &b, &a);

							GLuint& colByte = m_bgTexRGBA[x + (y * m_rayConfig.xResolution)];
							colByte |= (r << 0);
							colByte |= (g << 8);
							colByte |= (b << 16);
							colByte |= (a << 24);

							m_bgTexDepth[x + (y * m_rayConfig.xResolution)] = depth;
						}
					}
					roofXY += roofStep;
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
			eRayHit rayHit{ RAY_NOHIT };
			float distance{ 0.f };
			int wallNodeIndex{ 0 };
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
				if (mapCheck.x >= 0 && mapCheck.x < m_map.gridWidth && mapCheck.y >= 0 && mapCheck.y < m_map.gridHeight)
				{
					// if tile has assigned value 1, it EXISTS
					wallNodeIndex = mapCheck.x + (mapCheck.y * m_map.gridWidth);
					if (m_map.pNodes[wallNodeIndex].texIdWall != eLevelTextures::TEX_NONE)
					{
						rayHit = side ? RAY_HIT_SIDE : RAY_HIT_FRONT;
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
				int halfHeight{ static_cast<int>((m_rayConfig.yResolution / 2) / depth) };

				int mid{ static_cast<int>(m_frame.viewPitch + (m_rayConfig.yResolution / 2)) };
				int top{ mid - halfHeight };
				int bottom{ mid + halfHeight };
				const SDL_Surface* pWallTexture = pMapTextures->GetSDLSurface(m_map.pNodes[wallNodeIndex].texIdWall);
				ASSERT(pWallTexture);

				// Draw textured vertical line segments for wall
				bool bWallFinished = false;
				int renderedUp = 0;
				int renderedDown = 0;
				while (!bWallFinished)
				{
					for (int screenY = std::max(0, top); screenY < bottom; screenY++)
					{
						if (screenY >= m_rayConfig.yResolution)
						{
							break;
						}

						const int screenIndex{ screenX + (screenY * m_rayConfig.xResolution) };
						if (renderDepth < m_bgTexDepth[screenIndex])
						{
							// X Index into WallTexture = x position inside cell
							int texX = static_cast<int>((rayHit == RAY_HIT_FRONT ? (intersection.x - mapCheck.x) : (intersection.y - mapCheck.y)) * (pWallTexture->w - 1));
							if (texX < 0)
								texX += pWallTexture->w;
							// Y Index into WallTexture = percentage through current Y forloop
							int texY = (pWallTexture->h - 1) - static_cast<int>((static_cast<float>(screenY - top) / (bottom - top)) * (pWallTexture->h - 1));
							ASSERT(texX < pWallTexture->w && texX >= 0);
							ASSERT(texY < pWallTexture->h && texY >= 0);

							Uint32* pixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pWallTexture->pixels) + (texY * pWallTexture->pitch) + (texX * pWallTexture->format->BytesPerPixel));
							Uint8 r, g, b, a;
							SDL_GetRGBA(*pixel, pWallTexture->format, &r, &g, &b, &a);

							GLuint& colByte = m_bgTexRGBA[screenIndex];
							colByte = 0;
							colByte |= (r << 0);
							colByte |= (g << 8);
							colByte |= (b << 16);
							colByte |= (a << 24);

							m_bgTexDepth[screenIndex] = renderDepth;
						}
					}

					if (m_map.pNodes[wallNodeIndex].extendUp > renderedUp++)
					{
						top = (renderedUp * (halfHeight * 2)) + mid - halfHeight;
						bottom = (renderedUp * (halfHeight * 2)) + mid + halfHeight;
					}
					else if (m_map.pNodes[wallNodeIndex].extendDown > renderedDown++)
					{
						top = (-renderedDown * (halfHeight * 2)) + mid - halfHeight;
						bottom = (-renderedDown * (halfHeight * 2)) + mid + halfHeight;
					}
					else
					{
						bWallFinished = true;
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
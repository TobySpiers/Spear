#include "SpearEngine/Core.h"
#include "SpearEngine/ServiceLocator.h"
#include "SpearEngine/ScreenRenderer.h"
#include "SpearEngine/TextureArray.h"

#include "Raycaster.h"
#include "LevelManager.h"

#include <algorithm>

#if _DEBUG
#include "SpearEngine/FrameProfiler.h"
#endif

GLfloat* Raycaster::m_bgTexPixels{nullptr};
GLfloat* Raycaster::m_bgTexDepth{nullptr};
RaycastParams Raycaster::m_rayConfig;
MapData Raycaster::m_map;
GLfloat Raycaster::m_mapMaxDepth{FLT_MAX};

void Raycaster::RecreateBackgroundArrays(int width, int height)
{
	delete[] m_bgTexPixels;
	m_bgTexPixels = new GLfloat[width * height * m_bgTexPixelSize]; // Floor * Width * RGB
	std::fill(m_bgTexPixels, m_bgTexPixels + (width * height * m_bgTexPixelSize), GLfloat(0)); // init to 0

	delete[] m_bgTexDepth;
	m_bgTexDepth = new GLfloat[width * height];
	std::fill(m_bgTexDepth, m_bgTexDepth + (width * height), GLfloat(m_mapMaxDepth));
}

void Raycaster::ClearBackgroundArrays()
{
	std::fill(m_bgTexPixels, m_bgTexPixels + (m_rayConfig.xResolution * m_rayConfig.yResolution * m_bgTexPixelSize), GLfloat(0));
	std::fill(m_bgTexDepth, m_bgTexDepth + (m_rayConfig.xResolution * m_rayConfig.yResolution), GLfloat(m_mapMaxDepth));
}

void Raycaster::LoadLevel(const char* filename)
{
	LevelManager::LoadLevel(filename, m_map);
	if (m_bgTexPixels == nullptr)
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

void Raycaster::Draw2DWalls(const Vector2f& pos, const float angle, RaycastWall* pWalls, int wallCount)
{
	Spear::ScreenRenderer& rend = Spear::ServiceLocator::GetScreenRenderer();
	//rend.SetBatchLineWidth(1.0f);

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
		rend.AddRawLine(line, pWalls[i].colour);
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
		rend.AddRawLine(line, Colour4f::White());
	}
}

void Raycaster::Draw3DWalls(const Vector2f& pos, const float angle, RaycastWall* pWalls, int wallCount)
{
	Spear::ScreenRenderer& rend = Spear::ServiceLocator::GetScreenRenderer();
	int lineWidth{ static_cast<int>(Spear::Core::GetWindowSize().x) / m_rayConfig.xResolution };
	//rend.SetBatchLineWidth(lineWidth);

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
			rend.AddRawLine(line, rayColour);
		}
	}
}

void Raycaster::Draw2DGrid(const Vector2f& pos, const float angle)
{
	Spear::ScreenRenderer& rend = Spear::ServiceLocator::GetScreenRenderer();
	rend.SetBatchLineWidth(0, 2.0f);

	// Draw tiles
	for (int x = 0; x < m_map.gridWidth; x++)
	{
		for (int y = 0; y < m_map.gridHeight; y++)
		{
			Spear::ScreenRenderer::LinePolyData square;
			square.segments = 4;
			square.colour = m_map.pNodes[x + (y * m_map.gridWidth)].texIdWall != eLevelTextures::TEX_NONE ? Colour4f::Blue() : Colour4f::White();
			square.pos = Vector2f(x, y) + Vector2f(0.5, 0.5f);
			square.radius = 0.65f; // this is radius of each corner (not width/height)... sizing slightly under 0.707 for visual niceness
			square.rotation = TO_RADIANS(45.f);

			if (x == pos.ToInt().x && y == pos.ToInt().y)
			{
				square.colour = Colour4f::Green();
			}

			square.pos *= m_rayConfig.scale2D;
			square.radius *= m_rayConfig.scale2D;

			rend.AddLinePoly(square);
		}
	}

	// Draw player
	Spear::ScreenRenderer::LinePolyData poly;
	poly.colour = Colour4f::Red();
	poly.radius = 0.2f * m_rayConfig.scale2D;
	poly.segments = 3;
	poly.pos = pos * m_rayConfig.scale2D;
	poly.rotation = angle;
	rend.AddLinePoly(poly);

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
		Colour4f lineColour = Colour4f::White();
		if (tileFound)
		{
			rayEnd = rayStart + rayDir * distance;
			lineColour = Colour4f::Red();
		}
		line.start = pos * m_rayConfig.scale2D;
		line.end = rayEnd * m_rayConfig.scale2D;
		rend.AddRawLine(line, lineColour);
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
	
// CPU bound for now. Potential to eventualy convert into a shader...
void Raycaster::Draw3DGrid(const Vector2f& pos, const float angle)
{
	START_PROFILE("3D Raycaster");

	Spear::ScreenRenderer& rend = Spear::ServiceLocator::GetScreenRenderer();

	// Calculate our resolution
	float pixelWidth{ static_cast<float>(Spear::Core::GetWindowSize().x) / m_rayConfig.xResolution };
	float pixelHeight{ static_cast<float>(Spear::Core::GetWindowSize().y) / m_rayConfig.yResolution };
	rend.SetBatchLineWidth(0, pixelWidth * 2);

	// Screen Plane ray-distribution to avoid squash/stretch warping
	const float halfFov{ m_rayConfig.fieldOfView / 2 };
	const Vector2f forward{ Normalize(Vector2f(cos(angle), sin(angle))) };
	const Vector2f screenPlaneL{ pos + (Vector2f(cos(angle - halfFov), sin(angle - halfFov)) * m_rayConfig.farClip) };
	const Vector2f screenPlaneR{ pos + (Vector2f(cos(angle + halfFov), sin(angle + halfFov)) * m_rayConfig.farClip) };
	const Vector2f screenVector{ screenPlaneR - screenPlaneL };

	// Ray angle/spacing data
	const Vector2f raySpacingDir{ forward.Normal() * -1 };
	const float raySpacingLength{ screenVector.Length() / m_rayConfig.xResolution };

	// FLOOR/CEILING CASTING (SLOW)
	const Spear::TextureBase* pMapTextures = Spear::ServiceLocator::GetScreenRenderer().GetBatchTextures(0);
	for (int y = (m_rayConfig.yResolution / 2) + 1; y < m_rayConfig.yResolution; y++)
	{
		// Current y position compared to the center of the screen (the horizon)
		// Starts at 1, increases to HalfHeight
		int p = y - m_rayConfig.yResolution / 2;

		// Vertical position of the camera.
		float posZ = 0.625f * m_rayConfig.yResolution;

		// Horizontal distance from the camera to the floor for the current row.
		// 0.5 is the z position exactly in the middle between floor and ceiling.
		float rowDistance = posZ / p;

		// unit vectors representing directions to form left/right edge of FoV
		Vector2f planeDirLeft = Vector2f(cos(angle - halfFov), sin(angle - halfFov));
		Vector2f planeDirRight = Vector2f(cos(angle + halfFov), sin(angle + halfFov));

		// starting position of first pixel in row (left)
		// essentially the 'left-most ray' along a length (depth) of rowDistance
		Vector2f floorXY = pos + rowDistance * planeDirLeft;

		// vector representing position offset equivalent to 1 pixel right (imagine topdown 2D view, this 'jumps' horizontally by 1 ray)
		Vector2f floorStep = rowDistance * (planeDirRight - planeDirLeft) / m_rayConfig.xResolution;

		for (int x = 0; x < m_rayConfig.xResolution; ++x)
		{
			int cellX = (int)(floorXY.x);
			int cellY = (int)(floorXY.y);

			if(cellX >= 0 && cellY >= 0 && cellX < m_map.gridWidth && cellY < m_map.gridHeight)
			{
				// Calculate depth 
				float depth{ Projection(floorXY - pos, forward * m_rayConfig.farClip).Length() };
				depth /= m_mapMaxDepth;

				// Floor sampling
				if( m_map.pNodes[cellX + (cellY * m_map.gridWidth)].texIdFloor != eLevelTextures::TEX_NONE)
				{
					const SDL_Surface* pFloorTexture = pMapTextures->GetSDLSurface(m_map.pNodes[cellX + (cellY * m_map.gridWidth)].texIdFloor);
					ASSERT(pFloorTexture);

					int texX = static_cast<int>((floorXY.x - cellX) * pFloorTexture->w);
					int texY = static_cast<int>((floorXY.y - cellY) * pFloorTexture->h);
					if(texX < 0)
						texX += pFloorTexture->w;
					if(texY < 0)
						texY += pFloorTexture->h;

					ASSERT(texX < pFloorTexture->w);
					ASSERT(texY < pFloorTexture->h);
					Uint32* pixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pFloorTexture->pixels) + (texY * pFloorTexture->pitch) + (texX * pFloorTexture->format->BytesPerPixel));
					Uint8 r, g, b;
					SDL_GetRGB(*pixel, pFloorTexture->format, &r, &g, &b);

					m_bgTexPixels[(m_bgTexPixelSize * x) + ((m_bgTexPixelSize * (m_rayConfig.yResolution - y)) * m_rayConfig.xResolution) + 0] = (float)r / 255;
					m_bgTexPixels[(m_bgTexPixelSize * x) + ((m_bgTexPixelSize * (m_rayConfig.yResolution - y)) * m_rayConfig.xResolution) + 1] = (float)g / 255;
					m_bgTexPixels[(m_bgTexPixelSize * x) + ((m_bgTexPixelSize * (m_rayConfig.yResolution - y)) * m_rayConfig.xResolution) + 2] = (float)b / 255;

					m_bgTexDepth[x + ((m_rayConfig.yResolution - y) * m_rayConfig.xResolution)] = depth;
				}

				// Roof sampling
				if (m_map.pNodes[cellX + (cellY * m_map.gridWidth)].texIdRoof != eLevelTextures::TEX_NONE)
				{
					const SDL_Surface* pRoofTexture = pMapTextures->GetSDLSurface(m_map.pNodes[cellX + (cellY * m_map.gridWidth)].texIdRoof);
					ASSERT(pRoofTexture);

					int texX = static_cast<int>((floorXY.x - cellX) * pRoofTexture->w);
					int texY = static_cast<int>((floorXY.y - cellY) * pRoofTexture->h);
					if (texX < 0)
						texX += pRoofTexture->w;
					if (texY < 0)
						texY += pRoofTexture->h;

					ASSERT(texX < pRoofTexture->w);
					ASSERT(texY < pRoofTexture->h);
					Uint32* pixel = reinterpret_cast<Uint32*>(static_cast<Uint8*>(pRoofTexture->pixels) + (texY * pRoofTexture->pitch) + (texX * pRoofTexture->format->BytesPerPixel));
					Uint8 r, g, b;
					SDL_GetRGB(*pixel, pRoofTexture->format, &r, &g, &b);

					m_bgTexPixels[(m_bgTexPixelSize * x) + (m_bgTexPixelSize * y * m_rayConfig.xResolution) + 0] = (float)r / 255;
					m_bgTexPixels[(m_bgTexPixelSize * x) + (m_bgTexPixelSize * y * m_rayConfig.xResolution) + 1] = (float)g / 255;
					m_bgTexPixels[(m_bgTexPixelSize * x) + (m_bgTexPixelSize * y * m_rayConfig.xResolution) + 2] = (float)b / 255;

					m_bgTexDepth[x + (y * m_rayConfig.xResolution)] = depth;
				}
			}
			floorXY += floorStep;
		}
	}
	// Upload Floors+Ceilings background texture (SLOW)
	rend.SetBackgroundTextureData(m_bgTexPixels, m_bgTexDepth, m_rayConfig.xResolution, m_rayConfig.yResolution);
	ClearBackgroundArrays();

	// Using DDA (digital differential analysis) to quickly calculate intersections
	for (int screenX = 0; screenX < m_rayConfig.xResolution; screenX++)
	{
		Vector2f rayStart = pos;
		Vector2f rayEnd{ screenPlaneL - (raySpacingDir * raySpacingLength * screenX) };
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
		eRayHit rayHit{RAY_NOHIT};
		float distance{ 0.f };
		while (rayHit == RAY_NOHIT && distance < m_rayConfig.farClip)
		{
			bool side{false};
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
				if (m_map.pNodes[mapCheck.x + (mapCheck.y * m_map.gridWidth)].texIdWall != eLevelTextures::TEX_NONE)
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
			// Project THIS RAY onto the FORWARD VECTOR of the camera to get distance from camera with no fisheye distortion
			Vector2f intersection{rayStart + rayDir * distance};
			float depth{ Projection(intersection - pos, forward * m_rayConfig.farClip).Length() };

			float height{ (Spear::Core::GetWindowSize().y / 2.0f) / depth };
			float mid{ Spear::Core::GetWindowSize().y / 2.0f };

			Spear::ScreenRenderer::LineData line;
			line.start = Vector2f((screenX * pixelWidth), mid - height);
			line.end = Vector2f((screenX * pixelWidth), mid + height);
			line.start.y = line.start.y - fmod(line.start.y, pixelHeight);
			line.end.y = line.end.y - fmod(line.end.y, pixelHeight);
			line.texLayer = m_map.pNodes[mapCheck.x + (mapCheck.y * m_map.gridWidth)].texIdWall;
			line.depth = depth / m_mapMaxDepth;

			// UV X coord
			if (rayHit == RAY_HIT_FRONT)
			{
				int tileStart = static_cast<int>(intersection.x); // truncate to find start of tile
				line.texPosX = intersection.x - tileStart;
			}
			else
			{
				int tileStart = static_cast<int>(intersection.y); // truncate to find start of tile
				line.texPosX = intersection.y - tileStart;
			}
			rend.AddTexturedLine(line, 0);

			if (m_map.pNodes[mapCheck.x + (mapCheck.y * m_map.gridWidth)].extendUp)
			{
				Spear::ScreenRenderer::LineData upSection = line;

				upSection.end.y = upSection.start.y;
				upSection.start.y -= (height * 2);
				rend.AddTexturedLine(upSection, 0);
			}
			if (m_map.pNodes[mapCheck.x + (mapCheck.y * m_map.gridWidth)].extendDown)
			{
				line.start.y = line.end.y;
				line.end.y += (height * 2);
				rend.AddTexturedLine(line, 0);
			}
		}
	}

	END_PROFILE("3D Raycaster");
}
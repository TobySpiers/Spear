#include "Core.h"
#include "ServiceLocator.h"
#include "ScreenRenderer.h"

#include "Raycaster.h"

#include <algorithm>

namespace Spear
{
	GLfloat* Raycaster::m_bgTexPixels{nullptr};
	RaycastDDAGrid Raycaster::m_ddaGrid;
	RaycastParams Raycaster::m_rayConfig;

	void Raycaster::RecreateBackgroundArray(int width, int height)
	{
		delete[] m_bgTexPixels;
		m_bgTexPixels = new GLfloat[width * height * m_bgTexPixelSize]; // Floor * Width * RGB
		std::fill(m_bgTexPixels, m_bgTexPixels + (width * height * m_bgTexPixelSize), GLfloat(0)); // init to 0
	}

	void Raycaster::ClearBackgroundArray()
	{
		std::fill(m_bgTexPixels, m_bgTexPixels + (m_rayConfig.xResolution * m_rayConfig.yResolution * m_bgTexPixelSize), GLfloat(0));
	}

	void Raycaster::SubmitNewGrid(u8 width, u8 height, const s8* pWorldIds, const u8* pRoofIds)
	{
		// Resize if necessary
		if (width != m_ddaGrid.width && height != m_ddaGrid.height)
		{
			delete[] m_ddaGrid.pWorldIds;
			delete[] m_ddaGrid.pRoofIds;
			m_ddaGrid.pWorldIds = new s8[width * height];
			m_ddaGrid.pRoofIds = new u8[width * height];
			m_ddaGrid.width = width;
			m_ddaGrid.height = height;
		}

		// copy input data to stored grid
		memcpy(m_ddaGrid.pWorldIds, pWorldIds, width * height);
		memcpy(m_ddaGrid.pRoofIds, pRoofIds, width * height);

		// create background texture if one does not exist
		if (m_bgTexPixels == nullptr)
		{
			RecreateBackgroundArray(m_rayConfig.xResolution, m_rayConfig.yResolution);
		}
	}

	void Raycaster::ApplyConfig(const RaycastParams& config)
	{ 
		
		if (config.xResolution != m_rayConfig.xResolution || config.yResolution != m_rayConfig.yResolution)
		{
			RecreateBackgroundArray(config.xResolution, config.yResolution);
		}

		m_rayConfig = config; 
	};

	void Raycaster::Draw2DWalls(const Vector2f& pos, const float angle, RaycastWall* pWalls, int wallCount)
	{
		ScreenRenderer& rend = ServiceLocator::GetScreenRenderer();
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
			ScreenRenderer::LineData line;
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

			ScreenRenderer::LineData line;
			line.start = pos;
			line.end = foundIntersect? intersect : rayEndPoint;
			rend.AddRawLine(line, Colour4f::White());
		}
	}

	void Raycaster::Draw3DWalls(const Vector2f& pos, const float angle, RaycastWall* pWalls, int wallCount)
	{
		ScreenRenderer& rend = ServiceLocator::GetScreenRenderer();
		int lineWidth{ static_cast<int>(Core::GetWindowSize().x) / m_rayConfig.xResolution };
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

				float height{ (Core::GetWindowSize().y / 2.0f) / depth };
				float mid{ Core::GetWindowSize().y / 2.0f };

				ScreenRenderer::LineData line;
				line.start = Vector2f((screenX * lineWidth) + (lineWidth / 2), mid - height);
				line.end = Vector2f((screenX * lineWidth) + (lineWidth / 2), mid + height);
				rend.AddRawLine(line, rayColour);
			}
		}
	}

	void Raycaster::Draw2DGrid(const Vector2f& pos, const float angle)
	{
		ScreenRenderer& rend = ServiceLocator::GetScreenRenderer();
		rend.SetBatchLineWidth(0, 2.0f);

		// Draw tiles
		for (int x = 0; x < m_ddaGrid.width; x++)
		{
			for (int y = 0; y < m_ddaGrid.height; y++)
			{
				ScreenRenderer::LinePolyData square;
				square.segments = 4;
				square.colour = m_ddaGrid.pWorldIds[x + (y * m_ddaGrid.width)] ? Colour4f::Blue() : Colour4f::White();
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
		ScreenRenderer::LinePolyData poly;
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
				if(mapCheck.x >= 0 && mapCheck.x < m_ddaGrid.width && mapCheck.y >= 0 && mapCheck.y < m_ddaGrid.height)
				{	
					// if tile has assigned value above 0, it EXISTS
					if (m_ddaGrid.pWorldIds[mapCheck.x + (mapCheck.y * m_ddaGrid.width)] > 0) 
					{
						tileFound = true;
					}
				}
			}

			// ====================================
			// RENDER
			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			ScreenRenderer::LineData line;
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
		ScreenRenderer& rend = ServiceLocator::GetScreenRenderer();

		// Calculate our resolution
		float pixelWidth{ static_cast<float>(Core::GetWindowSize().x) / m_rayConfig.xResolution };
		float pixelHeight{ static_cast<float>(Core::GetWindowSize().y) / m_rayConfig.yResolution };
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

		// TEMP: Create a simple 'floor' texture (2x2 RGB)
		const int floorTexWidth{2};
		const int floorTexHeight{2};
		GLfloat floorTex[floorTexWidth * floorTexHeight * 3] = {
			1.f, 1.f, 1.f,	1.f, 1.f, 1.f,
			0.f, 0.f, 0.f,	0.f, 0.f, 0.f
		};

		// FLOOR/CEILING CASTING (SLOW)
		for (int y = m_rayConfig.yResolution / 2 + 1; y < m_rayConfig.yResolution; y++)
		{
			// Current y position compared to the center of the screen (the horizon)
			// Starts at 1, increases to HalfHeight
			int p = y - m_rayConfig.yResolution / 2;

			// Vertical position of the camera.
			float posZ = 0.605f * m_rayConfig.yResolution;

			// Horizontal distance from the camera to the floor for the current row.
			// 0.5 is the z position exactly in the middle between floor and ceiling.
			float rowDistance = posZ / p;

			// unit vectors representing directions to form left/right edge of FoV
			Vector2f planeDirLeft = Vector2f(cos(angle - halfFov), sin(angle - halfFov));
			Vector2f planeDirRight = Vector2f(cos(angle + halfFov), sin(angle + halfFov));

			// starting position of first pixel in row (left)
			// essentially the 'left-most ray' along a length (depth) of rowDistance
			Vector2f floorXY = pos + rowDistance * planeDirLeft;

			// vector representing position offset equivalent to 1 pixel right (imagine topdown 2D view, this 'jumps' over by 1 ray)
			Vector2f floorStep = rowDistance * (planeDirRight - planeDirLeft) / m_rayConfig.xResolution;

			for (int x = 0; x < m_rayConfig.xResolution; ++x)
			{
				int cellX = (int)(floorXY.x);
				int cellY = (int)(floorXY.y);

				// for now, just proving coordinate lookup by rendering WHITE for tiles with a value of 2
				if(cellX >= 0 && cellY >= 0 && cellX < m_ddaGrid.width && cellY < m_ddaGrid.height && m_ddaGrid.pWorldIds[cellX + (cellY * m_ddaGrid.width)] != 0)
				{
					m_bgTexPixels[(m_bgTexPixelSize * x) + (m_bgTexPixelSize * y * m_rayConfig.xResolution) + 0] = 1.f;
					m_bgTexPixels[(m_bgTexPixelSize * x) + (m_bgTexPixelSize * y * m_rayConfig.xResolution) + 1] = 1.f;
					m_bgTexPixels[(m_bgTexPixelSize * x) + (m_bgTexPixelSize * y * m_rayConfig.xResolution) + 2] = 1.f;
				}

				floorXY += floorStep;
			}
		}
		// Upload Floors+Ceilings background texture (SLOW)
		rend.SetBackgroundTextureData(m_bgTexPixels, m_rayConfig.xResolution, m_rayConfig.yResolution);
		ClearBackgroundArray();

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
				if (mapCheck.x >= 0 && mapCheck.x < m_ddaGrid.width && mapCheck.y >= 0 && mapCheck.y < m_ddaGrid.height)
				{
					// if tile has assigned value 1, it EXISTS
					if (m_ddaGrid.pWorldIds[mapCheck.x + (mapCheck.y * m_ddaGrid.width)] > 0)
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

				float height{ (Core::GetWindowSize().y / 2.0f) / depth };
				float mid{ Core::GetWindowSize().y / 2.0f };

				ScreenRenderer::LineData line;
				line.start = Vector2f((screenX * pixelWidth), mid - height);
				line.end = Vector2f((screenX * pixelWidth), mid + height);
				line.start.y = line.start.y - fmod(line.start.y, pixelHeight);
				line.end.y = line.end.y - fmod(line.end.y, pixelHeight);
				line.texLayer = m_ddaGrid.pWorldIds[mapCheck.x + (mapCheck.y * m_ddaGrid.width)] - 1;

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
			}
		}
	}
}
#include "Core.h"
#include "ServiceLocator.h"
#include "ScreenRenderer.h"

#include "Raycaster.h"

#include <algorithm>

namespace Spear
{
	void Raycaster::Draw2DWalls(const RaycastParams& param, RaycastWall* pWalls, int wallCount)
	{
		ScreenRenderer& rend = ServiceLocator::GetLineRenderer();
		rend.SetLineWidth(1.0f);

		// Define the 'screen'
		const float halfFov{ param.fieldOfView / 2 };
		const Vector2f forward{ Normalize(Vector2f(cos(param.rotation), sin(param.rotation))) };
		const Vector2f screenPlaneL{ param.pos + (Vector2f(cos(param.rotation + halfFov), sin(param.rotation + halfFov)) * param.farClip) };
		const Vector2f screenPlaneR{ param.pos + (Vector2f(cos(param.rotation - halfFov), sin(param.rotation - halfFov)) * param.farClip) };
		const Vector2f screenVector{ screenPlaneR - screenPlaneL };
		// Define the 'rays'
		const Vector2f raySpacingDir{ forward.Normal() * -1 };
		const float raySpacingLength{ screenVector.Length() / param.xResolution };

		// Draw each wall
		for (int i = 0; i < wallCount; i++)
		{
			ScreenRenderer::LineData line;
			line.start = pWalls[i].origin;
			line.end = pWalls[i].origin + pWalls[i].vec;
			line.colour = pWalls[i].colour;

			rend.AddLine(line);
		}

		// Draw each ray
		for (int i = 0; i < param.xResolution; i++)
		{
			Vector2f rayEndPoint{screenPlaneL + (raySpacingDir * raySpacingLength * i)};
			Vector2f ray{rayEndPoint - param.pos};

			// Check each wall...
			Vector2f intersect;
			bool foundIntersect{ false };
			for (int w = 0; w < wallCount; w++)
			{
				RaycastWall& wall = pWalls[w];

				// Check for an intersect...
				Vector2f result;
				if (VectorIntersection(param.pos, ray, wall.origin, wall.vec, result))
				{
					// Check if it was nearer than any previous discovered intersect...
					float distance{ Vector2f(param.pos - result).LengthSqr()};
					float existingDistance{ Vector2f(param.pos - intersect).LengthSqr() };
					if (!foundIntersect || distance < existingDistance)
					{
						// Store result
						intersect = result;
						foundIntersect = true;
					}
				}
			}

			ScreenRenderer::LineData line;
			line.start = param.pos;
			line.end = foundIntersect? intersect : rayEndPoint;
			line.colour = Colour4f::White();
			rend.AddLine(line);
		}
	}

	void Raycaster::Draw3DWalls(const RaycastParams& param, RaycastWall* pWalls, int wallCount)
	{
		ScreenRenderer& rend = ServiceLocator::GetLineRenderer();
		int lineWidth{ static_cast<int>(Core::GetWindowSize().x) / param.xResolution };
		rend.SetLineWidth(lineWidth);

		// Define a flat 'screen plane' to evenly distribute rays onto
		// This distribution ensures objects do not squash/stretch as they traverse the screen
		// If we don't do this, radial-distribution results in larger gaps between rays at further edges of flat surfaces
		const float halfFov{ param.fieldOfView / 2 };
		const Vector2f forward{ Normalize(Vector2f(cos(param.rotation), sin(param.rotation))) };
		const Vector2f screenPlaneL{ param.pos + (Vector2f(cos(param.rotation - halfFov), sin(param.rotation - halfFov)) * param.farClip) };
		const Vector2f screenPlaneR{ param.pos + (Vector2f(cos(param.rotation + halfFov), sin(param.rotation + halfFov)) * param.farClip) };
		const Vector2f screenVector{ screenPlaneR - screenPlaneL };

		// Define ray spacing
		const Vector2f raySpacingDir{ forward.Normal() * -1 };
		const float raySpacingLength{ screenVector.Length() / param.xResolution };

		// For each ray...
		for (int screenX = 0; screenX < param.xResolution; screenX++)
		{
			Vector2f rayEndPoint{ screenPlaneL - (raySpacingDir * raySpacingLength * screenX) };
			Vector2f ray{ rayEndPoint - param.pos };

			// Initial ray data
			float nearestLength{ param.farClip };
			Colour4f rayColour = Colour4f::Invisible();
			Vector2f intersection{0.f, 0.f};

			// Check each wall...
			for (int w = 0; w < wallCount; w++)
			{
				RaycastWall& wall = pWalls[w];

				// Check for an intersect...
				Vector2f result;
				if (VectorIntersection(param.pos, ray, wall.origin, wall.vec, result))
				{
					// If it was nearer than previous intersect...
					float newLength{ Vector2f(param.pos - result).Length() };
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
				float depth{ Projection(intersection - param.pos, forward * param.farClip).Length() };
				if (depth < param.nearClip)
				{
					continue;
				}
				rayColour.a = (1.f / (depth / 4));

				float height{ (Core::GetWindowSize().y / 2.0f) / depth };
				float mid{ Core::GetWindowSize().y / 2.0f };

				ScreenRenderer::LineData line;
				line.start = Vector2f((screenX * lineWidth) + (lineWidth / 2), mid - height);
				line.end = Vector2f((screenX * lineWidth) + (lineWidth / 2), mid + height);
				line.colour = rayColour;
				rend.AddLine(line);
			}
		}
	}

	void Raycaster::Draw2DGrid(const RaycastParams& param, RaycastDDAGrid* pGrid)
	{
		ScreenRenderer& rend = ServiceLocator::GetLineRenderer();
		rend.SetLineWidth(2.0f);

		// Draw tiles
		for (int x = 0; x < pGrid->width; x++)
		{
			for (int y = 0; y < pGrid->height; y++)
			{
				ScreenRenderer::LinePolyData square;
				square.segments = 4;
				square.colour = pGrid->pValues[x + (y * pGrid->width)] ? Colour4f::Blue() : Colour4f::White();
				square.pos = Vector2f(x, y) + Vector2f(0.5, 0.5f);
				square.radius = 0.65f; // this is radius of each corner (not width/height)... sizing slightly under 0.707 for visual niceness
				square.rotation = TO_RADIANS(45.f);

				if (x == param.pos.ToInt().x && y == param.pos.ToInt().y)
				{
					square.colour = Colour4f::Green();
				}

				square.pos *= param.scale2D;
				square.radius *= param.scale2D;

				rend.AddLinePoly(square);
			}
		}

		// Draw player
		ScreenRenderer::LinePolyData poly;
		poly.colour = Colour4f::Red();
		poly.radius = 0.2f * param.scale2D;
		poly.segments = 3;
		poly.pos = param.pos * param.scale2D;
		poly.rotation = param.rotation;
		rend.AddLinePoly(poly);

		// Draw rays
		const Vector2f forward{ Normalize(Vector2f(cos(param.rotation), sin(param.rotation))) };

		const float halfFov{ param.fieldOfView / 2 };
		Vector2f fovLeftExtent{ param.pos + (Vector2f(cos(param.rotation - halfFov), sin(param.rotation - halfFov)) * param.farClip) };
		Vector2f fovRightExtent{ param.pos + (Vector2f(cos(param.rotation + halfFov), sin(param.rotation + halfFov)) * param.farClip) };
		const Vector2f fovExtentWidth{ fovRightExtent - fovLeftExtent };

		const float raySpacing{ fovExtentWidth.Length() / param.xResolution };
		const Vector2f raySpacingDir{ forward.Normal() * -1 };

		// Using DDA (digital differential analysis) to quickly calculate intersections
		for(int x = 0; x < param.xResolution; x++)
		{
			Vector2f rayStart = param.pos;
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
			while (!tileFound && distance < param.farClip)
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
				if(mapCheck.x >= 0 && mapCheck.x < pGrid->width && mapCheck.y >= 0 && mapCheck.y < pGrid->height)
				{	
					// if tile has assigned value above 0, it EXISTS
					if (pGrid->pValues[mapCheck.x + (mapCheck.y * pGrid->width)] > 0) 
					{
						tileFound = true;
					}
				}
			}

			// ====================================
			// RENDER
			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			ScreenRenderer::LineData line;
			if (tileFound)
			{
				rayEnd = rayStart + rayDir * distance;
				line.colour = Colour4f::Red();
			}
			line.start = param.pos * param.scale2D;
			line.end = rayEnd * param.scale2D;
			rend.AddLine(line);
		}

		//=====================================================================================
		//FLOOR CASTING
		for (int y = param.yResolution / 2 + 1; y < param.yResolution; y++)
		{
			// Current y position compared to the center of the screen (the horizon)
			// Starts at 1, increases to HalfHeight
			int p = y - param.yResolution / 2;

			// Vertical position of the camera.
			float posZ = 0.61f * param.yResolution;

			// Horizontal distance from the camera to the floor for the current row.
			// 0.5 is the z position exactly in the middle between floor and ceiling.
			float rowDistance = posZ / p;

			// unit vectors representing directions to form left/right edge of FoV
			Vector2f planeDirLeft = Vector2f(cos(param.rotation - halfFov), sin(param.rotation - halfFov));
			Vector2f planeDirRight = Vector2f(cos(param.rotation + halfFov), sin(param.rotation + halfFov));

			// starting position of first pixel in row (left)
			// essentially the 'left-most ray' along a length (depth) of rowDistance
			Vector2f floorXY = param.pos + rowDistance * planeDirLeft;

			// vector representing position offset equivalent to 1 pixel right (imagine topdown 2D view, this 'jumps' over by 1 ray)
			Vector2f floorStep = rowDistance * (planeDirRight - planeDirLeft) / param.xResolution;

			for (int x = 0; x < param.xResolution; ++x)
			{
				int cellX = (int)(floorXY.x);
				int cellY = (int)(floorXY.y);

				// If we render floorX and floorY to the screen, we can view the positions on the 2D grid each floor texture gets sampled from!
				//ScreenRenderer::LinePolyData marker;
				//marker.colour = Colour4f::Red();
				//marker.radius = 5.f;
				//marker.pos = Vector2f(floorX, floorY) * param.scale2D;
				//marker.segments = 6;
				//rend.AddLinePoly(marker);

				floorXY += floorStep;
			}
		}
		//=====================================================================================

	}
	
	// CPU bound for now. Potential to eventualy convert into a shader...
	void Raycaster::Draw3DGrid(const RaycastParams& param, RaycastDDAGrid* pGrid)
	{
		ScreenRenderer& rend = ServiceLocator::GetLineRenderer();

		// Calculate our resolution
		float pixelWidth{ static_cast<float>(Core::GetWindowSize().x) / param.xResolution };
		float pixelHeight{ static_cast<float>(Core::GetWindowSize().y) / param.yResolution };
		rend.SetLineWidth(pixelWidth * 2);

		// Screen Plane ray-distribution to avoid squash/stretch warping
		const float halfFov{ param.fieldOfView / 2 };
		const Vector2f forward{ Normalize(Vector2f(cos(param.rotation), sin(param.rotation))) };
		const Vector2f screenPlaneL{ param.pos + (Vector2f(cos(param.rotation - halfFov), sin(param.rotation - halfFov)) * param.farClip) };
		const Vector2f screenPlaneR{ param.pos + (Vector2f(cos(param.rotation + halfFov), sin(param.rotation + halfFov)) * param.farClip) };
		const Vector2f screenVector{ screenPlaneR - screenPlaneL };

		// Ray angle/spacing data
		const Vector2f raySpacingDir{ forward.Normal() * -1 };
		const float raySpacingLength{ screenVector.Length() / param.xResolution };

		//=====================================================================================
		// TEMPORARY, TO REFACTOR: Create a 'background' texture (this will be target for floor/ceiling rendering)
		// #ToDo: make sure this is a persistent array (not allocating/destroying every frame)
		const int bgTexWidth{param.xResolution};
		const int bgTexHeight{param.yResolution};
		const int bgTexBytes{3}; // rgb
		GLfloat* bgPixelArray = new GLfloat[bgTexWidth * bgTexHeight * bgTexBytes]; // Floor * Width * RGB

		// Set RGB for corner pixel
		bgPixelArray[(bgTexBytes*0) + 0] = 1.f;
		bgPixelArray[(bgTexBytes*0) + 1] = 1.f;
		bgPixelArray[(bgTexBytes*0) + 2] = 1.f;

		// TEMP: Create a simple 'floor' texture (2x2 RGB)
		const int floorTexWidth{2};
		const int floorTexHeight{2};
		GLfloat floorTex[floorTexWidth * floorTexHeight * 3] = {
			1.f, 1.f, 1.f,	1.f, 1.f, 1.f,
			0.f, 0.f, 0.f,	0.f, 0.f, 0.f
		};

		//FLOOR CASTING
		for (int y = param.yResolution / 2 + 1; y < param.yResolution; y++)
		{
			// Current y position compared to the center of the screen (the horizon)
			// Starts at 1, increases to HalfHeight
			int p = y - param.yResolution / 2;

			// Vertical position of the camera.
			float posZ = 0.605f * param.yResolution;

			// Horizontal distance from the camera to the floor for the current row.
			// 0.5 is the z position exactly in the middle between floor and ceiling.
			float rowDistance = posZ / p;

			// unit vectors representing directions to form left/right edge of FoV
			Vector2f planeDirLeft = Vector2f(cos(param.rotation - halfFov), sin(param.rotation - halfFov));
			Vector2f planeDirRight = Vector2f(cos(param.rotation + halfFov), sin(param.rotation + halfFov));

			// starting position of first pixel in row (left)
			// essentially the 'left-most ray' along a length (depth) of rowDistance
			Vector2f floorXY = param.pos + rowDistance * planeDirLeft;

			// vector representing position offset equivalent to 1 pixel right (imagine topdown 2D view, this 'jumps' over by 1 ray)
			Vector2f floorStep = rowDistance * (planeDirRight - planeDirLeft) / param.xResolution;

			for (int x = 0; x < param.xResolution; ++x)
			{
				int cellX = (int)(floorXY.x);
				int cellY = (int)(floorXY.y);

				// for now, just proving coordinate lookup by rendering WHITE for tiles with a value of 2
				if(cellX >= 0 && cellY >= 0 && cellX < pGrid->width && cellY < pGrid->height && pGrid->pValues[cellX + (cellY * pGrid->width)] == 2)
				{
					bgPixelArray[(bgTexBytes * x) + (bgTexBytes * y * bgTexWidth) + 0] = 1.f;
					bgPixelArray[(bgTexBytes * x) + (bgTexBytes * y * bgTexWidth) + 1] = 1.f;
					bgPixelArray[(bgTexBytes * x) + (bgTexBytes * y * bgTexWidth) + 2] = 1.f;
				}

				floorXY += floorStep;
			}
		}

		// Using DDA (digital differential analysis) to quickly calculate intersections
		const float texStep{1.0f / param.texResolution};
		for (int screenX = 0; screenX < param.xResolution; screenX++)
		{
			Vector2f rayStart = param.pos;
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
			while (rayHit == RAY_NOHIT && distance < param.farClip)
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
				if (mapCheck.x >= 0 && mapCheck.x < pGrid->width && mapCheck.y >= 0 && mapCheck.y < pGrid->height)
				{
					// if tile has assigned value 1, it EXISTS
					if (pGrid->pValues[mapCheck.x + (mapCheck.y * pGrid->width)] == 1)
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
				float depth{ Projection(intersection - param.pos, forward * param.farClip).Length() };

				float height{ (Core::GetWindowSize().y / 2.0f) / depth };
				float mid{ Core::GetWindowSize().y / 2.0f };

				ScreenRenderer::LineData line;
				line.start = Vector2f((screenX * pixelWidth), mid - height);
				line.end = Vector2f((screenX * pixelWidth), mid + height);
				line.start.y = line.start.y - fmod(line.start.y, pixelHeight);
				line.end.y = line.end.y - fmod(line.end.y, pixelHeight);
				line.colour = rayHit == RAY_HIT_FRONT ? Colour4f::White() : Colour4f(0.9f, 0.9f, 0.9f, 1.0f);

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
				line.texPosX = line.texPosX - fmod(line.texPosX, texStep);

				rend.AddLine(line);
			}
		}


		// Update background texture (VERY SLOW)
		rend.SetBackgroundData(bgPixelArray, bgTexWidth, bgTexHeight);

		// #ToDo: turn into a member variable so we do not need to allocate/delete every frame
		delete[] bgPixelArray;
	}
}
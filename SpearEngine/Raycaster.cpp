#include "Core.h"
#include "ServiceLocator.h"
#include "LineRenderer.h"

#include "Raycaster.h"

#include <algorithm>

namespace Spear
{
	void Raycaster::Draw2DWalls(const RaycastParams& param, RaycastWall* pWalls, int wallCount)
	{
		LineRenderer& rend = ServiceLocator::GetLineRenderer();
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
			LineRenderer::LineData line;
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

			LineRenderer::LineData line;
			line.start = param.pos;
			line.end = foundIntersect? intersect : rayEndPoint;
			line.colour = Colour::White();
			rend.AddLine(line);
		}
	}

	void Raycaster::Draw3DWalls(const RaycastParams& param, RaycastWall* pWalls, int wallCount)
	{
		LineRenderer& rend = ServiceLocator::GetLineRenderer();
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
			Colour rayColour = Colour::Invisible();
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
				depth *= param.depthCorrection;
				rayColour.a = (1.f / (depth / 4));

				float height{ (Core::GetWindowSize().y / 2.0f) / depth };
				float mid{ Core::GetWindowSize().y / 2.0f };

				LineRenderer::LineData line;
				line.start = Vector2f((screenX * lineWidth) + (lineWidth / 2), mid - height);
				line.end = Vector2f((screenX * lineWidth) + (lineWidth / 2), mid + height);
				line.colour = rayColour;
				rend.AddLine(line);
			}
		}
	}

	void Raycaster::Draw2DGrid(const RaycastParams& param, RaycastDDAGrid* pGrid)
	{
		LineRenderer& rend = ServiceLocator::GetLineRenderer();
		rend.SetLineWidth(2.0f);

		// Draw tiles
		for (int x = 0; x < pGrid->width; x++)
		{
			for (int y = 0; y < pGrid->height; y++)
			{
				LineRenderer::LinePolyData square;
				square.segments = 4;
				square.colour = pGrid->pValues[x + (y * pGrid->width)] ? Colour::Blue() : Colour::White();
				square.pos = Vector2f(x, y) + Vector2f(0.5, 0.5f);
				square.radius = 0.65f; // this is radius of each corner (not width/height)... sizing slightly under 0.707 for visual niceness
				square.rotation = TO_RADIANS(45.f);

				if (x == param.pos.ToInt().x && y == param.pos.ToInt().y)
				{
					square.colour = Colour::Green();
				}

				square.pos *= param.scale2D;
				square.radius *= param.scale2D;

				rend.AddLinePoly(square);
			}
		}

		// Draw player
		LineRenderer::LinePolyData poly;
		poly.colour = Colour::Red();
		poly.radius = 0.2f * param.scale2D;
		poly.segments = 3;
		poly.pos = param.pos * param.scale2D;
		poly.rotation = param.rotation;
		rend.AddLinePoly(poly);

		// Draw rays
		const float halfFov{ param.fieldOfView / 2 };
		const Vector2f forward{ Normalize(Vector2f(cos(param.rotation), sin(param.rotation))) };
		const Vector2f screenPlaneL{ param.pos + (Vector2f(cos(param.rotation - halfFov), sin(param.rotation - halfFov)) * param.farClip) };
		const Vector2f screenPlaneR{ param.pos + (Vector2f(cos(param.rotation + halfFov), sin(param.rotation + halfFov)) * param.farClip) };
		const Vector2f screenVector{ screenPlaneR - screenPlaneL };

		const Vector2f raySpacingDir{ forward.Normal() * -1 };
		const float raySpacingLength{ screenVector.Length() / param.xResolution };

		// Using DDA (digital differential analysis) to quickly calculate intersections
		for(int x = 0; x < param.xResolution; x++)
		{
			Vector2f rayStart = param.pos;
			Vector2f rayEnd{ screenPlaneL - (raySpacingDir * raySpacingLength * x) };
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
			LineRenderer::LineData line;
			if (tileFound)
			{
				rayEnd = rayStart + rayDir * distance;
				line.colour = Colour::Red();
			}
			line.start = param.pos * param.scale2D;
			line.end = rayEnd * param.scale2D;
			rend.AddLine(line);
		}
	}
	
	void Raycaster::Draw3DGrid(const RaycastParams& param, RaycastDDAGrid* pGrid)
	{
		LineRenderer& rend = ServiceLocator::GetLineRenderer();

		// Calculate our resolution
		float pixelWidth{ static_cast<float>(Core::GetWindowSize().x) / param.xResolution };
		float pixelHeight {static_cast<float>(Core::GetWindowSize().y) / param.yResolution };
		rend.SetLineWidth(pixelWidth * 2);

		// Screen Plane ray-distribution to avoid squash/stretch warping
		const float halfFov{ param.fieldOfView / 2 };
		const Vector2f forward{ Normalize(Vector2f(cos(param.rotation), sin(param.rotation))) };
		const Vector2f screenPlaneL{ param.pos + (Vector2f(cos(param.rotation - halfFov), sin(param.rotation - halfFov)) * param.farClip) };
		const Vector2f screenPlaneR{ param.pos + (Vector2f(cos(param.rotation + halfFov), sin(param.rotation + halfFov)) * param.farClip) };
		const Vector2f screenVector{ screenPlaneR - screenPlaneL };

		const Vector2f raySpacingDir{ forward.Normal() * -1 };
		const float raySpacingLength{ screenVector.Length() / param.xResolution };

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
					// if tile has assigned value above 0, it EXISTS
					if (pGrid->pValues[mapCheck.x + (mapCheck.y * pGrid->width)] > 0)
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
				depth *= param.depthCorrection;

				float height{ (Core::GetWindowSize().y / 2.0f) / depth };
				float mid{ Core::GetWindowSize().y / 2.0f };

				LineRenderer::LineData line;
				line.start = Vector2f((screenX * pixelWidth), mid - height);
				line.end = Vector2f((screenX * pixelWidth), mid + height);
				line.start.y = line.start.y - fmod(line.start.y, pixelHeight);
				line.end.y = line.end.y - fmod(line.end.y, pixelHeight);
				line.colour = rayHit == RAY_HIT_FRONT ? Colour::White() : Colour(0.9f, 0.9f, 0.9f, 1.0f);

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
	}
}
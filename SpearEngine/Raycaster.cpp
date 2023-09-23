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
		const float raySpacingLength{ screenVector.Length() / param.resolution };

		// Draw each wall
		for (int i = 0; i < wallCount; i++)
		{
			LineData line;
			line.start = pWalls[i].origin;
			line.end = pWalls[i].origin + pWalls[i].vec;
			line.colour = pWalls[i].colour;

			rend.AddLine(line);
		}

		// Draw each ray
		for (int i = 0; i < param.resolution; i++)
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

			LineData line;
			line.start = param.pos;
			line.end = foundIntersect? intersect : rayEndPoint;
			line.colour = Colour::White();
			rend.AddLine(line);
		}
	}

	void Raycaster::Draw3DWalls(const RaycastParams& param, RaycastWall* pWalls, int wallCount)
	{
		LineRenderer& rend = ServiceLocator::GetLineRenderer();
		int lineWidth{ static_cast<int>(Core::GetWindowSize().x) / param.resolution };
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
		const float raySpacingLength{ screenVector.Length() / param.resolution };

		// For each ray...
		for (int screenX = 0; screenX < param.resolution; screenX++)
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

				LineData line;
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
		rend.SetLineWidth(1.0f);

		// Draw tiles
		for (int x = 0; x < pGrid->width; x++)
		{
			for (int y = 0; y < pGrid->height; y++)
			{
				LinePolyData square;
				square.segments = 4;
				square.colour = pGrid->pValues[x + (y * pGrid->width)] ? Colour::Blue() : Colour::White();
				square.pos = Vector2f(x * pGrid->tileDimension, y * pGrid->tileDimension) + Vector2f(pGrid->tileDimension / 2, pGrid->tileDimension / 2);
				square.radius = static_cast<float>(pGrid->tileDimension) * 0.65f; // this is radius of each corner (not width/height)... sizing slightly under 0.707 for visual niceness
				square.rotation = TO_RADIANS(45.f);

				square.pos *= param.scale2D;
				square.radius *= param.scale2D;

				rend.AddLinePoly(square);
			}
		}

		// Draw player
		Spear::LinePolyData poly;
		poly.colour = Colour::Red();
		poly.radius = 0.2f * param.scale2D;
		poly.segments = 3;
		poly.pos = param.pos * param.scale2D;
		poly.rotation = param.rotation;
		rend.AddLinePoly(poly);

		// Draw rays
		//Vector2f
	}
	
	void Raycaster::Draw3DGrid(const RaycastParams& param, RaycastDDAGrid* pGrid)
	{
		// NOTE: need to rework param.pos so that player positions are directly equivalent to pGrid index values
		// player.pos.x = 7.33 == pGrid.tile[7]

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
		const float raySpacingLength{ screenVector.Length() / param.resolution };

		// For each ray...
		for (int screenX = 0; screenX < param.resolution; screenX++)
		{
			Vector2f rayEndPoint{ screenPlaneL - (raySpacingDir * raySpacingLength * screenX) };
			Vector2f rayDir{ Normalize(rayEndPoint - param.pos) };
			Vector2f rayStepSize{  sqrt(1 + (rayDir.y / rayDir.x) * (rayDir.y / rayDir.x)),	// scalar for 1 unit X
									sqrt(1 + (rayDir.x / rayDir.y) * (rayDir.x / rayDir.y))		// scalar for 1 unit Y
			};

			//int tileX{ (param.pos.x - fmod(param.pos.x, pGrid->tileDimension)) / pGrid->tileDimension };
			//int tileY{ (param.pos.y - fmod(param.pos.y, pGrid->tileDimension)) / pGrid->tileDimension };

			float rayLengthX;
			float rayStepX;

			float rayLengthY;
			float rayStepY;

			// Initialise Step Direction
			if(rayDir.x < 0)
			{
				rayStepX = -1;
				//rayLengthX = (param.pos.x - rayStepPos.x) * unitStepSize.x;
			}
			else
			{
				rayStepX = 1;
			}
			if (rayDir.y < 0)
			{
				rayStepY = -1;
			}
			else
			{
				rayStepY = 1;
			}
		}
	}
}
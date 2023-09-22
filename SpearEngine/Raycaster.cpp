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
		const Vector2D forward{ Normalize(Vector2D(cos(param.rotation), sin(param.rotation))) };
		const Vector2D screenPlaneL{ param.pos + (Vector2D(cos(param.rotation + halfFov), sin(param.rotation + halfFov)) * param.farClip) };
		const Vector2D screenPlaneR{ param.pos + (Vector2D(cos(param.rotation - halfFov), sin(param.rotation - halfFov)) * param.farClip) };
		const Vector2D screenVector{ screenPlaneR - screenPlaneL };
		// Define the 'rays'
		const Vector2D raySpacingDir{ forward.Normal() * -1 };
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
			Vector2D rayEndPoint{screenPlaneL + (raySpacingDir * raySpacingLength * i)};
			Vector2D ray{rayEndPoint - param.pos};

			// Check each wall...
			Vector2D intersect;
			bool foundIntersect{ false };
			for (int w = 0; w < wallCount; w++)
			{
				RaycastWall& wall = pWalls[w];

				// Check for an intersect...
				Vector2D result;
				if (VectorIntersection(param.pos, ray, wall.origin, wall.vec, result))
				{
					// Check if it was nearer than any previous discovered intersect...
					float distance{ Vector2D(param.pos - result).LengthSqr()};
					float existingDistance{ Vector2D(param.pos - intersect).LengthSqr() };
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
		const Vector2D forward{ Normalize(Vector2D(cos(param.rotation), sin(param.rotation))) };
		const Vector2D screenPlaneL{ param.pos + (Vector2D(cos(param.rotation - halfFov), sin(param.rotation - halfFov)) * param.farClip) };
		const Vector2D screenPlaneR{ param.pos + (Vector2D(cos(param.rotation + halfFov), sin(param.rotation + halfFov)) * param.farClip) };
		const Vector2D screenVector{ screenPlaneR - screenPlaneL };

		// Define ray spacing
		const Vector2D raySpacingDir{ forward.Normal() * -1 };
		const float raySpacingLength{ screenVector.Length() / param.resolution };

		// For each ray...
		for (int screenX = 0; screenX < param.resolution; screenX++)
		{
			Vector2D rayEndPoint{ screenPlaneL - (raySpacingDir * raySpacingLength * screenX) };
			Vector2D ray{ rayEndPoint - param.pos };

			// Initial ray data
			float nearestLength{ param.farClip };
			Colour rayColour = Colour::Invisible();
			Vector2D intersection{0.f, 0.f};

			// Check each wall...
			for (int w = 0; w < wallCount; w++)
			{
				RaycastWall& wall = pWalls[w];

				// Check for an intersect...
				Vector2D result;
				if (VectorIntersection(param.pos, ray, wall.origin, wall.vec, result))
				{
					// If it was nearer than previous intersect...
					float newLength{ Vector2D(param.pos - result).Length() };
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
				line.start = Vector2D((screenX * lineWidth) + (lineWidth / 2), mid - height);
				line.end = Vector2D((screenX * lineWidth) + (lineWidth / 2), mid + height);
				line.colour = rayColour;
				rend.AddLine(line);
			}
		}
	}
}
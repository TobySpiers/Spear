#include "Core.h"
#include "ServiceLocator.h"
#include "LineRenderer.h"

#include "Raycaster.h"

#include <algorithm>

namespace Spear
{
	void RaycastWall::Draw()
	{
		LineRenderer& rend = ServiceLocator::GetLineRenderer();
		
		LineData line;
		line.start = origin;
		line.end = origin + vec;
		line.colour = colour;

		rend.AddLine(line);
	}

	void Raycaster::Draw2D(const RaycastParams& param)
	{
		LineRenderer& rend = ServiceLocator::GetLineRenderer();

		float anglePerRay{param.fieldOfView / param.resolution};

		// For each ray
		for (int i = 0; i < param.resolution; i++)
		{

			Vector2D ray{ sin(param.rotation + (anglePerRay * i)), cos(param.rotation + (anglePerRay * i)) };
			ray *= param.viewDistance;

			// Check each wall...
			Vector2D intersect;
			bool foundIntersect{false};
			for (int w = 0; w < param.wallCount; w++)
			{
				RaycastWall& wall = param.pWalls[w];
				
				// Check for an intersect...
				Vector2D result;
				if (VectorIntersection(param.pos, ray, wall.origin, wall.vec, result))
				{
					// Check if it was nearer than any previous discovered intersect...
					float distance{Vector2D(param.pos - result).LengthSqr()};
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
			line.end = foundIntersect ? intersect : param.pos + ray;
			line.colour = White;
			rend.AddLine(line);
		}
	}

	void Raycaster::Draw3D(const RaycastParams& param)
	{
		LineRenderer& rend = ServiceLocator::GetLineRenderer();

		float anglePerRay{ param.fieldOfView / param.resolution };

		// For each ray
		for (int i = 0; i < param.resolution; i++)
		{

			Vector2D ray{ sin(param.rotation + (anglePerRay * i)), cos(param.rotation + (anglePerRay * i)) };
			ray *= param.viewDistance;

			// Check each wall...
			Vector2D intersect;
			Colour intersectColour;
			bool foundIntersect{ false };
			for (int w = 0; w < param.wallCount; w++)
			{
				RaycastWall& wall = param.pWalls[w];

				// Check for an intersect...
				Vector2D result;
				if (VectorIntersection(param.pos, ray, wall.origin, wall.vec, result))
				{
					// Check if it was nearer than any previous discovered intersect...
					float distance{ Vector2D(param.pos - result).LengthSqr() };
					float existingDistance{ Vector2D(param.pos - intersect).LengthSqr() };
					if (!foundIntersect || distance < existingDistance)
					{
						// Store result
						intersect = result;
						intersectColour = wall.colour;
						foundIntersect = true;
					}
				}
			}

			int lineWidth{ static_cast<int>(Core::GetWindowSize().x) / param.resolution };
			int xScreenPos{ static_cast<int>(Core::GetWindowSize().x) - (i * lineWidth)};

			const float nearDistance{50.f};
			const float farDistance{2000.f};

			Spear::ServiceLocator::GetLineRenderer().SetLineWidth(lineWidth);
			if (foundIntersect)
			{
				// could optimise this by using Squared length value
				float clampedDistance = std::max(std::min(Vector2D(param.pos - intersect).Length(), farDistance), nearDistance);
				float percent{1 - ((clampedDistance - nearDistance) / farDistance)};
				float height = Core::GetWindowSize().y * percent;
				float mid{ Core::GetWindowSize().y / 2 };

				LineData line;
				line.start = Vector2D(xScreenPos, mid - (height / 2));
				line.end = Vector2D(xScreenPos, mid + (height / 2));
				line.colour = intersectColour;
				rend.AddLine(line);
			}
		}
	}

}
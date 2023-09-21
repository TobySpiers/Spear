#include "Core.h"
#include "ServiceLocator.h"
#include "LineRenderer.h"

#include "Raycaster.h"

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

		float anglePerRay{param.fieldOfView / param.rayCount};

		// For each ray
		for (int i = 0; i < param.rayCount; i++)
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

	}

}
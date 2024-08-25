#include "LevelData.h"

const GridNode* MapData::GetNode(Vector2i index)
{
	return GetNode(index.x, index.y);
}

const GridNode* MapData::GetNode(int x, int y)
{
	if (x >= 0 && x < gridWidth && y >= 0 && y < gridHeight)
	{
		return &pNodes[x + (y * gridWidth)];
	}
	return nullptr;
}

bool MapData::CollisionSearchDDA(const Vector2f& start, const Vector2f& trajectory, u8 collisionTestMask, Vector2f* out_hitPos, bool* out_bVerticalHit)
{
	if (out_hitPos)
	{
		*out_hitPos = start + trajectory;
	}

	const float distanceLimit{ trajectory.Length() };
	const Vector2f end{ start + trajectory };
	const Vector2f direction = Normalize(end - start);
	const Vector2f stepSize{sqrt(1 + (direction.y / direction.x) * (direction.y / direction.x)),	// length required to travel 1 X unit in Ray Direction
							sqrt(1 + (direction.x / direction.y) * (direction.x / direction.y)) };	// length required to travel 1 Y unit in Ray Direction

	Vector2i mapCheck = start.ToInt(); // truncation will 'snap' position to tile
	Vector2f testProgression; // tracks accumulating length of 'test' ray: via x units, via y units

	// 1. Calculate initial direction/distance values
	Vector2i step;
	if (direction.x < 0)
	{
		step.x = -1;
		testProgression.x = (start.x - static_cast<float>(mapCheck.x)) * stepSize.x;
	}
	else
	{
		step.x = 1;
		testProgression.x = (static_cast<float>(mapCheck.x + 1) - start.x) * stepSize.x;

	}
	if (direction.y < 0)
	{
		step.y = -1;
		testProgression.y = (start.y - static_cast<float>(mapCheck.y)) * stepSize.y;
	}
	else
	{
		step.y = 1;
		testProgression.y = (static_cast<float>(mapCheck.y + 1) - start.y) * stepSize.y;
	}

	// 2. Search step-by-step for a collision
	float distance{ 0.f };
	while (distance < distanceLimit)
	{
		if (testProgression.x < testProgression.y)
		{
			// X distance is currently shortest, increase X
			mapCheck.x += step.x;
			distance = testProgression.x;
			testProgression.x += stepSize.x; // increase ray by 1 X unit

			if (out_bVerticalHit)
			{
				*out_bVerticalHit = false;
			}
		}
		else
		{
			// Y distance is currently shortest, increase Y
			mapCheck.y += step.y;
			distance = testProgression.y;
			testProgression.y += stepSize.y; // increase ray by 1 Y unit

			if (out_bVerticalHit)
			{
				*out_bVerticalHit = true;
			}
		}

		// If we've exceeded our distance limit prior to finding a collision, then requested trace param does not collide
		if (distance > distanceLimit)
		{
			return false;
		}

		// Check position is within range of array
		if (const GridNode* node = GetNode(mapCheck))
		{
			// Test 'collision' based on critera
			if (node->collisionMask & collisionTestMask)
			{
				if (out_hitPos)
				{
					*out_hitPos = start + (direction * distance);
				}
				return true;
			}
		}
	}

	return false;
}

void GridNode::Reset()
{
	texIdRoof = eLevelTextures::TEX_NONE;
	texIdWall = eLevelTextures::TEX_NONE;
	texIdFloor = eLevelTextures::TEX_NONE;
	extendUp = 0;
	extendDown = 0;
	collisionMask = 0;
}

void EditorMapData::ClearData()
{
	for (int i = 0; i < MAP_WIDTH_MAX_SUPPORTED * MAP_HEIGHT_MAX_SUPPORTED; i++)
	{
		gridNodes[i].Reset();
	}
}
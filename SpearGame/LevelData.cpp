#include "LevelData.h"

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
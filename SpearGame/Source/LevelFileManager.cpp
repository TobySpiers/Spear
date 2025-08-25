#include "LevelFileManager.h"
#include "GameObject/GameObject.h"
#include "Collision/CollisionSystem.h"
#include <GameObject/OBoxCollider.h>

std::string GetFilePath(const char* levelName) {return std::string("../Assets/MAPS/") + std::string(levelName); };

char LevelFileManager::m_reservedMapMemory[MAP_RESERVED_BYTES];

void LevelFileManager::EditorSaveLevel(const EditorMapData& rMapData)
{
	std::ofstream file(GetFilePath(rMapData.mapName.c_str()) + ".level");

	// Write width/height header
	file	<< rMapData.gridWidth << std::endl
			<< rMapData.gridHeight << std::endl;

	// Write PlayerStart
	Serialize(rMapData.playerStart, file);

	// Write grid to file
	for (int x = 0; x < rMapData.gridWidth; x++)
	{
		for (int y = 0; y < rMapData.gridHeight; y++)
		{
			const GridNode& node = rMapData.gridNodes[(y * MAP_WIDTH_MAX_SUPPORTED) + x];
			Serialize(node, file);
		}
	}

	// Write map name
	file << rMapData.mapName << std::endl;

	// Write floor/roof heights
	for (int i = 0; i < PLANE_HEIGHTS_TOTAL; i++)
	{
		file << rMapData.planeHeights[i] << std::endl;
	}

	// Write GameObjects
	GameObject::GlobalSerialize(file);

	// Write Tileset/Spriteset
	file << std::endl;
	file << rMapData.tileDirectory.path().string() << std::endl;
	file << rMapData.spriteDirectory.path().string() << std::endl;

	// 'Other' settings
	file << rMapData.darkness << std::endl;
}

void LevelFileManager::EditorLoadLevel(const char* levelName, EditorMapData& rMapData)
{
	std::ifstream file(GetFilePath(levelName));

	// Read width/height header
	std::string width, height;
	std::getline(file, width);
	std::getline(file, height);
	rMapData.gridWidth = std::stoi(width);
	rMapData.gridHeight = std::stoi(height);

	// Read PlayerStart
	Deserialize(rMapData.playerStart, file);

	// Read grid
	for (int x = 0; x < rMapData.gridWidth; x++)
	{
		for (int y = 0; y < rMapData.gridHeight; y++)
		{
			GridNode& node = rMapData.gridNodes[(y * MAP_WIDTH_MAX_SUPPORTED) + x];
			Deserialize(node, file);
		}
	}

	// Read map name
	std::getline(file, rMapData.mapName);

	// Read floor/roof heights
	std::string planeHeight;
	for (int i = 0; i < PLANE_HEIGHTS_TOTAL; i++)
	{
		std::getline(file, planeHeight);
		rMapData.planeHeights[i] = std::stoi(planeHeight);
	}

	// Read GameObjects
	GameObject::GlobalDeserialize(file);

	// Read Tileset/Spriteset
	std::string temp;
	std::getline(file, temp);
	std::getline(file, temp);
	rMapData.tileDirectory = std::filesystem::directory_entry(temp);
	std::getline(file, temp);
	rMapData.spriteDirectory = std::filesystem::directory_entry(temp);

	// Read 'Other' settings
	std::getline(file, temp);
	rMapData.darkness = std::stoi(temp);
}

void LevelFileManager::LoadLevel(const char* levelName, MapData& rMapData)
{
	// Wipe any and all reserved memory to 0
	std::fill(std::begin(m_reservedMapMemory), std::end(m_reservedMapMemory), '\0');

	// Read width/height header
	std::ifstream file(GetFilePath(levelName));
	std::string fWidth, fHeight;
	std::getline(file, fWidth);
	std::getline(file, fHeight);
	rMapData.gridWidth = std::stoi(fWidth);
	rMapData.gridHeight = std::stoi(fHeight);
	ASSERT(rMapData.gridWidth > 0 && rMapData.gridWidth <= MAP_WIDTH_MAX_SUPPORTED && rMapData.gridHeight > 0 && rMapData.gridHeight <= MAP_HEIGHT_MAX_SUPPORTED);

	Deserialize(rMapData.playerStart, file);

	// Allocate rMapData.pNodes to use appropriate size in reserved memory
	// TODO: Ensure pNodes has an appropriate 'delete' call
	rMapData.pNodes = new (m_reservedMapMemory) GridNode[rMapData.gridWidth * rMapData.gridHeight];

	// Read grid
	std::string texFloor, texWall, texRoof, extendUp, extendDown, collMask;
	for (int x = 0; x < rMapData.gridWidth; x++)
	{
		for (int y = 0; y < rMapData.gridHeight; y++)
		{
			GridNode& node = rMapData.pNodes[(y * rMapData.gridWidth) + x];
			Deserialize(node, file);
		}
	}

	// Read map name
	std::getline(file, rMapData.mapName);

	// Read floor/roof heights
	std::string planeHeight;
	for (int i = 0; i < PLANE_HEIGHTS_TOTAL; i++)
	{
		std::getline(file, planeHeight);
		rMapData.planeHeights[i] = std::stoi(planeHeight);
	}

	// Initialise Collision QuadTree to appropriate size for level
	Collision::CollisionSystem2D::Get().ResizeQuadTree(Vector2f::ZeroVector, Vector2f(rMapData.gridWidth, rMapData.gridHeight));

	// Load GameObjects
	GameObject::GlobalDeserialize(file);

	// Generate collisions for tilemap
	// TODO: Detect and combine edges into single appropriately-sized AABBs
	for (int x = 0; x < rMapData.gridWidth; x++)
	{
		for (int y = 0; y < rMapData.gridHeight; y++)
		{
			if (const GridNode* mapNode = rMapData.GetNode({ x, y }))
			{
				if (mapNode->collisionMask)
				{
					OBoxCollider* coll = GameObject::Create<OBoxCollider>(Vector3f(x + 0.5f, y + 0.5f, 0));

					// Since player performs PreCheckedMovement against tiles, collision system does not need to test tiles against the player, hence removing the flag
					coll->ApplySetup(Collision::World, mapNode->collisionMask & ~Collision::Player, Collision::PROFILE_None, true);

					coll->SetHalfExtent({.5f, .5f});
				}
			}
		}
	}

	// Read Tileset/Spriteset
	std::string temp;
	std::getline(file, temp);
	std::getline(file, temp);
	rMapData.tileDirectory = std::filesystem::directory_entry(temp);
	std::getline(file, temp);
	rMapData.spriteDirectory = std::filesystem::directory_entry(temp);

	// Read 'Other' settings
	std::getline(file, temp);
	rMapData.darkness = std::stoi(temp);
}
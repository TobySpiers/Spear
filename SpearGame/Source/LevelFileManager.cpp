#include "LevelFileManager.h"


std::string GetFilePath(const char* levelName) {return std::string("../Assets/MAPS/") + std::string(levelName); };

char LevelFileManager::m_reservedMapMemory[MAP_RESERVED_BYTES];

void LevelFileManager::EditorSaveLevel(const EditorMapData& rMapData)
{
	std::ofstream file(GetFilePath(rMapData.mapName.c_str()) + ".level");

	// Write width/height header
	file	<< rMapData.gridWidth << std::endl
			<< rMapData.gridHeight << std::endl;

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

	file << rMapData.mapName;
}

void LevelFileManager::EditorLoadLevel(const char* levelName, EditorMapData& rMapData)
{
	// Read width/height header
	std::ifstream file(GetFilePath(levelName));
	std::string width, height;
	std::getline(file, width);
	std::getline(file, height);
	rMapData.gridWidth = std::stoi(width);
	rMapData.gridHeight = std::stoi(height);

	Deserialize(rMapData.playerStart, file);

	// Read grid
	std::string texFloor, texWall, texRoof, extendUp, extendDown, collMask;
	for (int x = 0; x < rMapData.gridWidth; x++)
	{
		for (int y = 0; y < rMapData.gridHeight; y++)
		{
			GridNode& node = rMapData.gridNodes[(y * MAP_WIDTH_MAX_SUPPORTED) + x];
			Deserialize(node, file);
		}
	}

	std::getline(file, rMapData.mapName);
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
}
#include "LevelManager.h"
#include "LevelData.h"
#include <fstream>

std::string GetFilePath(const char* levelName) {return std::string("../Assets/MAPS/") + std::string(levelName) + ".dat"; };

void LevelManager::EditorSaveLevel(const char* levelName, const EditorMapData& rMapData)
{
	std::ofstream fileStream(GetFilePath(levelName));

	fileStream	<< rMapData.gridWidth << std::endl
				<< rMapData.gridHeight << std::endl;

	// Write grid to file
	for (int x = 0; x < rMapData.gridWidth; x++)
	{
		for (int y = 0; y < rMapData.gridHeight; y++)
		{
			const GridNode& node = rMapData.gridNodes[(y * MAP_WIDTH_MAX_SUPPORTED) + x];
			fileStream	<< node.texIdFloor << std::endl
						<< node.texIdWall << std::endl
						<< node.texIdRoof << std::endl
						<< node.extendUp << std::endl
						<< node.extendDown << std::endl
						<< node.collisionMask << std::endl;
		}
	}
}

void LevelManager::EditorLoadLevel(const char* levelName, EditorMapData& rMapData)
{
	rMapData.ClearData();

	std::ifstream file(GetFilePath(levelName));
	std::string width, height;
	std::getline(file, width);
	std::getline(file, height);

	rMapData.gridWidth = std::stoi(width);
	rMapData.gridHeight = std::stoi(height);

	std::string texFloor, texWall, texRoof, extendUp, extendDown, collMask;
	for (int x = 0; x < rMapData.gridWidth; x++)
	{
		for (int y = 0; y < rMapData.gridHeight; y++)
		{
			std::getline(file, texFloor);
			std::getline(file, texWall);
			std::getline(file, texRoof);
			std::getline(file, extendUp);
			std::getline(file, extendDown);
			std::getline(file, collMask);

			GridNode& node = rMapData.gridNodes[(y * MAP_WIDTH_MAX_SUPPORTED) + x];
			node.texIdFloor = std::stoi(texFloor);
			node.texIdWall = std::stoi(texWall);
			node.texIdRoof = std::stoi(texRoof);
			node.extendUp = std::stoi(extendUp);
			node.extendDown = std::stoi(extendDown);
			node.collisionMask = std::stoi(collMask);
		}
	}
}

void LevelManager::LoadLevel(const char* levelName, MapData& rMapData)
{
	std::ifstream file(GetFilePath(levelName));
	std::string fWidth, fHeight;
	std::getline(file, fWidth);
	std::getline(file, fHeight);


	rMapData.gridWidth = std::stoi(fWidth);
	rMapData.gridHeight = std::stoi(fHeight);
	ASSERT(rMapData.gridWidth > 0 && rMapData.gridWidth < MAP_WIDTH_MAX_SUPPORTED && rMapData.gridHeight > 0 && rMapData.gridHeight < MAP_HEIGHT_MAX_SUPPORTED);
	rMapData.pNodes = new GridNode[rMapData.gridWidth * rMapData.gridHeight];

	std::string texFloor, texWall, texRoof, extendUp, extendDown, collMask;
	for (int x = 0; x < rMapData.gridWidth; x++)
	{
		for (int y = 0; y < rMapData.gridHeight; y++)
		{
			std::getline(file, texFloor);
			std::getline(file, texWall);
			std::getline(file, texRoof);
			std::getline(file, extendUp);
			std::getline(file, extendDown);
			std::getline(file, collMask);

			//GridNode& node = rMapData.pNodes[(y * rMapData.gridWidth) + x];
			//node.texIdFloor = std::stoi(texFloor);
			//node.texIdWall = std::stoi(texWall);
			//node.texIdRoof = std::stoi(texRoof);
			//node.extendUp = std::stoi(extendUp);
			//node.extendDown = std::stoi(extendDown);
			//node.collisionMask = std::stoi(collMask);
		}
	}
}
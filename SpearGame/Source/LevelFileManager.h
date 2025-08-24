#pragma once
#include "LevelData.h"
#include <fstream>

struct EditorMapData;
struct MapData;

class LevelFileManager
{
public:
	static void EditorSaveLevel(const EditorMapData& rMapData);
	static void EditorLoadLevel(const char* levelName, EditorMapData& rMapData);

	// For loading a level from scratch, with default GameObject states and generating collision objects for tiles
	static void LoadLevel(const char* levelName, MapData& rMapData);

	// TODO: For loading a savefile, file should contain map name, load tiles but skip loading objects/generating collision objects, then deserialise GameObjects from savefile instead of from level
	// static void LoadSavedGame(const char* saveName, MapData& rMapData);

private:
	// editor map data includes large gaps in array to allow for easy map-resizing
	// at game time, contiguously allocate maps into this reserved memory instead
	static const int MAP_RESERVED_BYTES{ (MAP_WIDTH_MAX_SUPPORTED * MAP_HEIGHT_MAX_SUPPORTED) * sizeof(GridNode) };
	static char m_reservedMapMemory[MAP_RESERVED_BYTES];

	template <typename T>
	static void Serialize(const T& data, std::ofstream& os)
	{
		os.write(reinterpret_cast<const char*>(&data), sizeof(T));
	}

	template <typename T>
	static void Deserialize(T& data, std::ifstream& is)
	{
		is.read(reinterpret_cast<char*>(&data), sizeof(T));
	}
};
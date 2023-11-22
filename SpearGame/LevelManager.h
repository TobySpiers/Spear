#pragma once

struct EditorMapData;
struct MapData;

class LevelManager
{
public:
	static void EditorSaveLevel(const char* levelName, const EditorMapData& rMapData);
	static void EditorLoadLevel(const char* levelName, EditorMapData& rMapData);

	static void LoadLevel(const char* levelName, MapData& rMapData);

	// reserve 100x100 gridnodes of memory, allocate using this when loading levels
};
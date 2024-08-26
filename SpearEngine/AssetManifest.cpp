#include "AssetManifest.h"
#include <queue>
#include <fstream>
#include <iostream>
#include <filesystem>

namespace Spear
{
	std::string GetManifestPath(const char* dir) { return std::string(dir) + "/manifest.dat"; };

	bool AssetManifest::GetManifest(const char* dir, const char* extension, Manifest& out_manifest)
	{
		out_manifest.clear();

		// Load manifest containing prior known files (allows loaded indexes to match values saved inside level.dats after file removals/additions)
		std::queue<int> manifestFreeSlots;
		{
			std::ifstream file(GetManifestPath(dir));
			if (file.is_open())
			{
				std::string sFileCount;
				std::getline(file, sFileCount);
				int fileCount{ std::stoi(sFileCount) };

				for (int i = 0; i < fileCount; i++)
				{
					out_manifest.push_back(std::string());
					std::getline(file, out_manifest.back());

					// If file no longer exists, mark this as a free slot we can replace with any newly added files (preserves order)
					if (!std::filesystem::exists(out_manifest.back()))
					{
						manifestFreeSlots.push(i);
					}
				}
			}
		}

		// Iterate directory for new files; use these to replace any missing files, otherwise append to back of list
		for (const std::filesystem::directory_entry& filepath : std::filesystem::directory_iterator(dir))
		{
			if (filepath.path().extension() == extension)
			{
				const std::string sPath = filepath.path().string();
				bool bIsNew{ true };
				for (std::string& knownPath : out_manifest)
				{
					if (knownPath == sPath)
					{
						bIsNew = false;
						break;
					}
				}

				if (bIsNew)
				{
					if (manifestFreeSlots.size())
					{
						out_manifest[manifestFreeSlots.front()] = sPath;
						manifestFreeSlots.pop();
					}
					else
					{
						out_manifest.push_back(sPath);
					}
				}
			}
		}
		if (!out_manifest.size() || out_manifest.size() == manifestFreeSlots.size())
		{
			return false;
		}

		// Save updated manifest
		{
			std::ofstream file(GetManifestPath(dir));
			file << out_manifest.size() << std::endl;
			for (const std::string& entry : out_manifest)
			{
				file << entry << std::endl;
			}
		}

		// Nullify any unreplaced free slots
		while (manifestFreeSlots.size())
		{
			out_manifest[manifestFreeSlots.front()] = "";
			manifestFreeSlots.pop();
		}

		return true;
	}
}
#pragma once
#include <vector>
#include <string>

namespace Spear
{
	using Manifest = std::vector<std::string>;

	class AssetManifest
	{
	public:
		// Returns false if no files with extension found
		static bool GetManifest(const char* dir, const char* extension, Manifest& out_manifest);
	};
}

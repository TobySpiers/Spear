#pragma once
#include "Core/Serializer.h"
#include "imgui.h"
#include <fstream>
#include <filesystem>

namespace Spear
{
	// A serializable property representing an asset inside a specific folder with optional extension, with visual support for ImGui
	class AssetProperty
	{
	public:
		AssetProperty(const char* directory, const char* extension) : m_directory(directory), m_extension(extension) {}

		friend std::ofstream& operator<<(std::ofstream& stream, const AssetProperty& obj);

		friend std::ifstream& operator>>(std::ifstream& stream, AssetProperty& obj);

		AssetProperty& operator<<(ExposedPropertyData& property);

		const std::filesystem::directory_entry& GetFilePath() const {return m_filepath; }

		AssetProperty& operator<<(PropertyManipulator& inserter);

		const AssetProperty& operator>>(PropertyManipulator& deleter) const;

		bool IsValid() {return m_filepath.path().string() != "None"; }

	private:
		const char* m_directory{nullptr};
		const char* m_extension{nullptr};
		std::filesystem::directory_entry m_filepath{"None"};
	};
}
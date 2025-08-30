#include "AssetProperty.h"

namespace Spear
{

	std::ofstream& Spear::operator<<(std::ofstream& stream, const AssetProperty& obj)
	{
		stream << obj.m_filepath.path().string().c_str() << " ";
		return stream;
	}

	std::ifstream& Spear::operator>>(std::ifstream& stream, AssetProperty& obj)
	{
		std::string temp;
		stream >> temp;
		obj.m_filepath = std::filesystem::directory_entry(temp);
		return stream;
	}

	AssetProperty& AssetProperty::operator<<(ExposedPropertyData& property)
	{
		std::filesystem::directory_entry cachedFilepath{ m_filepath };
		if (ImGui::BeginCombo(property.propertyName.c_str(), m_filepath.path().filename().string().c_str()))
		{
			if (ImGui::Selectable("None", m_filepath.path().string() == "None"))
			{
				m_filepath = std::filesystem::directory_entry("None");
			}
			if (m_filepath.path().string() == "None")
			{
				ImGui::SetItemDefaultFocus();
			}

			for (const std::filesystem::directory_entry& filepath : std::filesystem::directory_iterator(m_directory))
			{
				const std::string extension = filepath.path().extension().string();
				if (!m_extension || extension == m_extension)
				{
					bool bIsSelected = filepath == m_filepath;
					if (ImGui::Selectable(filepath.path().filename().string().c_str(), &bIsSelected))
					{
						if (filepath != m_filepath)
						{
							m_filepath = filepath;
						}
					}
					if (bIsSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}
			}

			ImGui::EndCombo();
		}

		if (cachedFilepath != m_filepath)
		{
			property.propertyChain.push_back(0);
			property.modifiedPropertyName = property.propertyName;
			property.SetOldValue(cachedFilepath);
			property.SetNewValue(m_filepath);
		}

		return *this;
	}

	AssetProperty& AssetProperty::operator<<(PropertyManipulator& inserter)
	{
		Serializer::SetPropertyInternal<std::filesystem::directory_entry>(inserter.value, m_filepath, inserter.outPropertyData);
		return *this;
	}

	const AssetProperty& AssetProperty::operator>>(PropertyManipulator& deleter) const
	{
		Serializer::DeletePropertyDataInternal<std::filesystem::directory_entry>(*deleter.allocatedValue);
		return *this;
	}
}
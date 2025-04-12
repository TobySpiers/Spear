#include "Serializer.h"
#include "GameObject/GameObject.h"
#include <imgui.h>
#include <imgui_stdlib.h>

void Serializer::Serialize(std::ofstream& stream, const std::string& string)
{
	stream << string.size() << string;
}

void Serializer::Deserialize(std::ifstream& stream, std::string& string)
{
    string.clear();
    size_t size;
    stream >> size;

    for (size_t i = 0; i < size; i++)
    {
        std::string::value_type character;
        stream >> character;
        string += character;
    }
}

bool Serializer::ExposeCategory(const char* category)
{
    return ImGui::TreeNodeEx(category, ImGuiTreeNodeFlags_DefaultOpen);
}

void Serializer::PopCategory()
{
    ImGui::TreePop();
}

std::string Serializer::GetPropertyName(const char* propertyNames, int propertyIndex)
{
    int strIndex{0};
    int strLength{0};

    int commaCount{0};
    int c{-1};
    while (propertyNames[++c] != '\0')
    {
        if (propertyNames[c] == ',')
        {
            commaCount++;

            if (commaCount == propertyIndex)
            {
                // Skip the comma and preceding whitespace if not end-of-string
                if (propertyNames[c + 1] != '\0' && propertyNames[c + 2] != '\0')
                {
                    c += 2;
                    strIndex = c;
                }

            }
            else if(commaCount > propertyIndex)
            {
                strLength = c - strIndex;
                break;
            }
        }
    }
    if (strLength == 0)
    {
        strLength = c;
    }
    
    return std::string(propertyNames + strIndex, strLength);
}

void Serializer::Expose(const char* propertyName, int& data)
{
    ImGui::InputInt(propertyName, &data);
}

void Serializer::Expose(const char* propertyName, float& data)
{
    ImGui::InputFloat(propertyName, &data);
}

void Serializer::Expose(const char* propertyName, bool& data)
{
    ImGui::Checkbox(propertyName, &data);
}

void Serializer::Expose(const char* propertyName, std::string& data)
{
    ImGui::InputText(propertyName, &data);
}

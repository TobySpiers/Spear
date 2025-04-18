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

const void* Serializer::GetProperty(const std::vector<int>& propertyChain, int step, const int& arg)
{
    return &arg;
}

const void* Serializer::GetProperty(const std::vector<int>& propertyChain, int step, const float& arg)
{
    return &arg;
}

const void* Serializer::GetProperty(const std::vector<int>& propertyChain, int step, const double& arg)
{
    return &arg;
}

const void* Serializer::GetProperty(const std::vector<int>& propertyChain, int step, const bool& arg)
{
    return &arg;
}

const void* Serializer::GetProperty(const std::vector<int>& propertyChain, int step, const std::string& arg)
{
    return &arg;
}

void Serializer::SetProperty(const void* newValue, const std::vector<int>& propertyChain, int step, int& arg)
{
    arg = *static_cast<const int*>(newValue);
}

void Serializer::SetProperty(const void* newValue, const std::vector<int>& propertyChain, int step, float& arg)
{
    arg = *static_cast<const float*>(newValue);
}

void Serializer::SetProperty(const void* newValue, const std::vector<int>& propertyChain, int step, double& arg)
{
    arg = *static_cast<const double*>(newValue);
}

void Serializer::SetProperty(const void* newValue, const std::vector<int>& propertyChain, int step, bool& arg)
{
    arg = *static_cast<const bool*>(newValue);
}

void Serializer::SetProperty(const void* newValue, const std::vector<int>& propertyChain, int step, std::string& arg)
{
    arg = *static_cast<const std::string*>(newValue);
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

bool Serializer::Expose(std::vector<int>& outPropertyChain, const char* propertyName, int& data)
{
    return ImGui::InputInt(propertyName, &data);
}

bool Serializer::Expose(std::vector<int>& outPropertyChain, const char* propertyName, float& data)
{
    return ImGui::InputFloat(propertyName, &data);
}

bool Serializer::Expose(std::vector<int>& outPropertyChain, const char* propertyName, double& data)
{
    return ImGui::InputDouble(propertyName, &data);
}

bool Serializer::Expose(std::vector<int>& outPropertyChain, const char* propertyName, bool& data)
{
    return ImGui::Checkbox(propertyName, &data);
}

bool Serializer::Expose(std::vector<int>& outPropertyChain, const char* propertyName, std::string& data)
{
    return ImGui::InputText(propertyName, &data);
}

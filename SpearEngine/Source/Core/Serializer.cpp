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

void Serializer::DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step, const int& arg)
{
    DeletePropertyDataInternal<int>(allocatedData);
}

void Serializer::DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step, const float& arg)
{
    DeletePropertyDataInternal<float>(allocatedData);
}

void Serializer::DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step, const double& arg)
{
    DeletePropertyDataInternal<double>(allocatedData);
}

void Serializer::DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step, const bool& arg)
{
    DeletePropertyDataInternal<bool>(allocatedData);
}

void Serializer::DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step, const std::string& arg)
{
    DeletePropertyDataInternal<std::string>(allocatedData);
}

void Serializer::SetProperty(const void* newValue, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step, int& arg)
{
    SetPropertyInternal(newValue, arg, outPropertyData);
}

void Serializer::SetProperty(const void* newValue, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step, float& arg)
{
    SetPropertyInternal(newValue, arg, outPropertyData);
}

void Serializer::SetProperty(const void* newValue, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step, double& arg)
{
    SetPropertyInternal(newValue, arg, outPropertyData);
}

void Serializer::SetProperty(const void* newValue, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step, bool& arg)
{
    SetPropertyInternal(newValue, arg, outPropertyData);
}

void Serializer::SetProperty(const void* newValue, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step, std::string& arg)
{
    SetPropertyInternal(newValue, arg, outPropertyData);
}

bool Serializer::ExposeCategory(const char* category)
{
    return ImGui::TreeNodeEx(category, ImGuiTreeNodeFlags_DefaultOpen);
}

void Serializer::PopCategory()
{
    ImGui::TreePop();
}

void Serializer::PushEnumVal(int enumVal, std::vector<int>& outVals)
{
    outVals.emplace_back(enumVal);
}

void Serializer::BreakPropertyNames(const char* propertyNames, std::vector<std::string>& outNames)
{
    outNames.clear();

    int cachedPos{ 0 };
    int c{ -1 };
    while (propertyNames[++c] != '\0')
    {
        if (propertyNames[c] == ',')
        {
            outNames.emplace_back(propertyNames + cachedPos, c - cachedPos);

            // Skip the comma and preceding whitespace if not end-of-string
            if (propertyNames[c + 1] != '\0' && propertyNames[c + 2] != '\0')
            {
                c += 2;
                cachedPos = c;
            }
        }
    }
    if (c - 1 != cachedPos)
    {
        outNames.emplace_back(propertyNames + cachedPos, c - cachedPos);
    }
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

bool Serializer::Expose(ExposedPropertyData& propertyData, const char* propertyName, int& data)
{
    int cachedData = data;
    if (ImGui::InputInt(propertyName, &data))
    {
        if (!propertyData.IsModifying())
        {
            propertyData.SetOldValue(cachedData);
            propertyData.modifiedPropertyName = propertyName;
        }
    }
    if(ImGui::IsItemDeactivatedAfterEdit())
    {
        propertyData.SetNewValue(data);
        return true;
    }
    return false;
}

bool Serializer::Expose(ExposedPropertyData& propertyData, const char* propertyName, float& data)
{
    float cachedData = data;
    if (ImGui::InputFloat(propertyName, &data))
    {
        if (!propertyData.IsModifying())
        {
            propertyData.SetOldValue(cachedData);
            propertyData.modifiedPropertyName = propertyName;
        }
    }
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        propertyData.SetNewValue(data);
        return true;
    }
    return false;
}

bool Serializer::Expose(ExposedPropertyData& propertyData, const char* propertyName, double& data)
{
    double cachedData = data;
    if (ImGui::InputDouble(propertyName, &data))
    {
        if (!propertyData.IsModifying())
        {
            propertyData.SetOldValue(cachedData);
            propertyData.modifiedPropertyName = propertyName;
        }
    }
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        propertyData.SetNewValue(data);
        return true;
    }
    return false;
}

bool Serializer::Expose(ExposedPropertyData& propertyData, const char* propertyName, bool& data)
{
    bool cachedData = data;
    if (ImGui::Checkbox(propertyName, &data))
    {
        propertyData.SetOldValue(cachedData);
        propertyData.SetNewValue(data);
        propertyData.modifiedPropertyName = propertyName;
        return true;
    }
    return false;
}

bool Serializer::Expose(ExposedPropertyData& propertyData, const char* propertyName, std::string& data)
{
    std::string cachedData = data;
    if (ImGui::InputText(propertyName, &data))
    {
        if(!propertyData.IsModifying())
        {
            propertyData.SetOldValue(cachedData);
            propertyData.modifiedPropertyName = propertyName;
        }
    }
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        propertyData.SetNewValue(data);
        return true;
    }
    return false;
}

ExposedPropertyData::ExposedPropertyData(GameObject* object)
    : m_object(object)
{ }

void ExposedPropertyData::Expose()
{
    m_object->PopulateEditorPanel(*this);
}

void ExposedPropertyData::CleanUp()
{
    if (WasModified())
    {
        m_object->DeletePropertyData(m_oldValue, propertyChain);
        m_object->DeletePropertyData(m_newValue, propertyChain);
    }
    ASSERT(m_oldValue == nullptr);
    ASSERT(m_newValue == nullptr);
}

ModifiedPropertyData::ModifiedPropertyData(GameObject* object, const ExposedPropertyData& refProperty)
    : m_object(object), m_propertyChain(&refProperty.propertyChain)
{}

void ModifiedPropertyData::CleanUp()
{
    if (m_oldValue != nullptr)
    {
        m_object->DeletePropertyData(m_oldValue, *m_propertyChain);
    }
    ASSERT(m_oldValue == nullptr);
}

ExposedEnumBase& ExposedEnumBase::operator<<(ExposedPropertyData& editor)
{
    ExposeEnum(editor);
    return *this;
}

void Serializer::ExposeEnum(ExposedPropertyData& propertyData, int& enumValue, const char* valuesString, const std::vector<int>& enumVals)
{
    std::vector<std::string> enumNames;
    BreakPropertyNames(valuesString, enumNames);
    ASSERT(enumNames.size() == enumVals.size());

    int previewIndex{-1};
    int i{0};
    for (int val : enumVals)
    {
        if (val == enumValue)
        {
            previewIndex = i;
            break;
        }
        i++;
    }

    int cachedOldVal = enumValue;
    if (ImGui::BeginCombo(propertyData.propertyName.c_str(), previewIndex < enumNames.size() ? enumNames[previewIndex].c_str() : "UNKNOWN"))
    {
        for (int i = 0; i < enumNames.size(); i++)
        {
            bool bIsSelected = enumValue == enumVals[i];
            if (ImGui::Selectable(enumNames[i].c_str(), bIsSelected))
            {
                enumValue = enumVals[i];
            }
            if (bIsSelected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }

        ImGui::EndCombo();
    }

    if (cachedOldVal != enumValue)
    {
        propertyData.SetOldValue(cachedOldVal);
        propertyData.SetNewValue(enumValue);
        propertyData.modifiedPropertyName = propertyData.propertyName;
        propertyData.propertyChain.emplace_back(0);
    }
}

void Serializer::ExposeEnumFlag(ExposedPropertyData& propertyData, int& flagContainer, int flagbit, const char* flagNames, int flagNameIndex)
{
    int cachedValue = flagContainer;
    std::string flagname = GetPropertyName(flagNames, flagNameIndex);
    if (ImGui::CheckboxFlags(flagname.c_str(), &flagContainer, flagbit))
    {
        propertyData.SetOldValue(cachedValue);
        propertyData.SetNewValue(flagContainer);
        propertyData.modifiedPropertyName = propertyData.propertyName + "::" + flagname;
        propertyData.propertyChain.emplace_back(0);
    }
}

ExposedEnumBase& ExposedEnumBase::operator<<(PropertyManipulator& inserter)
{
    Serializer::SetPropertyInternal<int>(inserter.value, m_val, inserter.outPropertyData);
    return *this;
}

const ExposedEnumBase& ExposedEnumBase::operator>>(PropertyManipulator& deleter) const
{
    Serializer::DeletePropertyDataInternal<int>(*deleter.allocatedValue);
    return *this;
}
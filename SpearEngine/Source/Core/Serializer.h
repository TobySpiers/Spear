#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "GameObject/GameObject.h"

// Macros to simplify needing lambda functions to correctly resolve Serialize/Deserialize overloads
#define SERIALIZE(outStream, ...) Serializer::ForwardArgsToSerializers([&](auto& arg) { Serializer::Serialize(outStream, arg); }, __VA_ARGS__);
#define DESERIALIZE(inStream, ...) Serializer::ForwardArgsToSerializers([&](auto& arg) { Serializer::Deserialize(inStream, arg); }, __VA_ARGS__);

// Macros to simplify exposing serialized variables in the editor
#define EXPOSE_PROPERTIES(propertyData, ...)\
int i{0}; Serializer::ForwardArgsToSerializers([&](auto& arg) { Serializer::ExposeInEditor(propertyData, #__VA_ARGS__, i++, arg); }, __VA_ARGS__);
#define SET_PROPERTY(newValue, propertyChain, outPropertyData, step, propertyId, ...)\
int i{0}; Serializer::ForwardArgsToSerializers([&](auto& arg) { if(i++ == propertyId) { Serializer::SetProperty(newValue, propertyChain, outPropertyData, step, arg); }}, __VA_ARGS__);
#define DELETE_PROPERTY_DATA(allocatedData, propertyChain, step, propertyId, ...)\
int i{0}; Serializer::ForwardArgsToSerializers([&](auto& arg) { if(i++ == propertyId) { Serializer::DeletePropertyData(allocatedData, propertyChain, step, arg); }}, __VA_ARGS__);

// Macro adding serialization support via insertion/extraction operators ( << & >> )
#define SERIALIZABLE_BASE(Class, ...)\
virtual void Serialize(std::ofstream& stream) const\
{\
    SERIALIZE(stream, __VA_ARGS__)\
}\
virtual void Deserialize(std::ifstream& stream)\
{\
    DESERIALIZE(stream, __VA_ARGS__)\
}\
virtual void PostDeserialize(){};\
friend std::ofstream& operator<<(std::ofstream& stream, const Class& obj)\
{\
    obj.Serialize(stream);\
    return stream;\
}\
friend std::ifstream& operator>>(std::ifstream& stream, Class& obj)\
{\
    obj.Deserialize(stream);\
    return stream;\
}\
virtual void ExposeToEditor(ExposedPropertyData& propertyData)\
{\
    if(Serializer::ExposeCategory(#Class))\
    {\
        EXPOSE_PROPERTIES(propertyData, __VA_ARGS__);\
        Serializer::PopCategory();\
    }\
}\
virtual void SetProperty(const void* newValue, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step)\
{\
	const int propertyId = propertyChain[propertyChain.size() - step];\
    ASSERT(propertyId != -1);\
	SET_PROPERTY(newValue, propertyChain, outPropertyData, step + 1, propertyId, __VA_ARGS__);\
}\
virtual void DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step) const\
{\
    const int propertyId = propertyChain[propertyChain.size() - step];\
    ASSERT(propertyId != -1);\
    DELETE_PROPERTY_DATA(allocatedData, propertyChain, step + 1, propertyId, __VA_ARGS__);\
}\
Class& operator<<(ExposedPropertyData& editor)\
{\
    ExposeToEditor(editor);\
    return *this;\
}\
Class& operator<<(PropertyManipulator& inserter)\
{\
    SetProperty(inserter.value, *inserter.propertyChain, inserter.outPropertyData, inserter.step);\
    return *this;\
}\
const Class& operator>>(PropertyManipulator& deleter) const\
{\
    DeletePropertyData(*deleter.allocatedValue, *deleter.propertyChain, deleter.step);\
    return *this;\
}\

// Macro adding serialization support via insertion/extraction operators ( << & >> ) for classes derived from a SERIALIZABLE_BASE class
#define SERIALIZABLE_DERIVED(Class, BaseClass, ...)\
virtual void Serialize(std::ofstream& stream) const override\
{\
    SERIALIZE(stream, __VA_ARGS__)\
    BaseClass::Serialize(stream);\
}\
virtual void Deserialize(std::ifstream& stream) override\
{\
    DESERIALIZE(stream, __VA_ARGS__)\
    BaseClass::Deserialize(stream);\
}\
friend std::ofstream& operator<<(std::ofstream& stream, const Class& obj)\
{\
    obj.Serialize(stream);\
    return stream;\
}\
friend std::ifstream& operator>>(std::ifstream& stream, Class& obj)\
{\
    obj.Deserialize(stream);\
    obj.PostDeserialize();\
    return stream;\
}\
virtual void ExposeToEditor(ExposedPropertyData& propertyData) override\
{\
    if(Serializer::ExposeCategory(#Class))\
    {\
        EXPOSE_PROPERTIES(propertyData, __VA_ARGS__);\
        const size_t cachedSize = propertyData.propertyChain.size();\
        BaseClass::ExposeToEditor(propertyData);\
        if(propertyData.propertyChain.size() > cachedSize)\
        {\
            propertyData.propertyChain.emplace_back(-1);\
        }\
        Serializer::PopCategory();\
    }\
}\
virtual void SetProperty(const void* newValue, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step) override\
{\
	const int propertyId = propertyChain[propertyChain.size() - step];\
    if(propertyId == -1)\
	{\
		BaseClass::SetProperty(newValue, propertyChain, outPropertyData, step + 1);\
        return;\
	}\
	SET_PROPERTY(newValue, propertyChain, outPropertyData, step + 1, propertyId, __VA_ARGS__);\
}\
virtual void DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step) const\
{\
    const int propertyId = propertyChain[propertyChain.size() - step];\
    if(propertyId == -1)\
	{\
		BaseClass::DeletePropertyData(allocatedData, propertyChain, step + 1);\
        return;\
	}\
    DELETE_PROPERTY_DATA(allocatedData, propertyChain, step + 1, propertyId, __VA_ARGS__);\
}\
Class& operator<<(ExposedPropertyData& editor)\
{\
    ExposeToEditor(editor);\
    return *this;\
}\
Class& operator<<(PropertyManipulator& inserter)\
{\
    SetProperty(inserter.value, *inserter.propertyChain, inserter.outPropertyData, inserter.step);\
    return *this;\
}\
const Class& operator>>(PropertyManipulator& deleter) const\
{\
    DeletePropertyData(*deleter.allocatedValue, *deleter.propertyChain, deleter.step);\
    return *this;\
}\

struct GameObjectPtrBase;

// Used to generically expose GameObject properties to ImGui panels
class ExposedPropertyData
{
public:
    ExposedPropertyData(GameObject* object);

    std::string propertyName;
    std::vector<int> propertyChain;
    std::string modifiedPropertyName;

    template <typename T>
    void SetOldValue(const T& oldValue)
    {
        ASSERT(!m_oldValue);
        m_oldValue = new T(oldValue);
        ASSERT(m_oldValue);
    };

    template <typename T>
    void SetNewValue(const T& newValue)
    {
        ASSERT(!m_newValue);
        m_newValue = new T(newValue);
        ASSERT(m_newValue);
    };

    bool WasModified() const {return m_newValue != nullptr || m_oldValue != nullptr;}
    GameObject* GetObject() const {ASSERT(m_object); return m_object;}
    const void* GetOldValue() const {ASSERT(m_oldValue); return m_oldValue;}
    const void* GetNewValue() const {ASSERT(m_newValue); return m_newValue;}
    void CleanUp();

private:
    GameObject* m_object{nullptr};

    // Heap allocated - CleanUp must be called to destroy correctly!
    const void* m_oldValue{nullptr};
    const void* m_newValue{nullptr};
};

class ModifiedPropertyData
{
public:
    ModifiedPropertyData(GameObject* object, const ExposedPropertyData& refProperty);

    template <typename T>
    void SetOldValue(const T& oldValue)
    {
        ASSERT(!m_oldValue);
        m_oldValue = new T(oldValue);
        ASSERT(m_oldValue);
    };

    GameObject* GetObject() const {ASSERT(m_object); return m_object;}
    const void* GetOldValue() const {ASSERT(m_oldValue); return m_oldValue;}
    void CleanUp();

private:
    GameObject* m_object{nullptr};
    const std::vector<int>* m_propertyChain{nullptr};

    // Heap allocated - CleanUp must be called to destroy correctly!
    const void* m_oldValue{nullptr};
};

// Used to generically get/set property values in Level Editor using '<<' and '>>' operators
class PropertyManipulator
{
public:
    const std::vector<int>* propertyChain;
    int step{1};
    const void* value{nullptr};
    const void** allocatedValue{nullptr};
    ModifiedPropertyData* outPropertyData{nullptr};
};

namespace Serializer
{
    // Variadic template function using fold expression to forward each argument to appropriate 'Serialize' function overload
    template<typename Func, typename... Args>
    void ForwardArgsToSerializers(Func func, Args&&... args)
    {
        (func(std::forward<Args>(args)), ...);
    }

    // Serialization generic types
    template <typename T>
    void Serialize(std::ofstream& ostream, const T& data)
    {
        ostream << data << " ";
    };
    template <typename T>
    void Deserialize(std::ifstream& istream, T& data)
    {
        istream >> data;
    }
    // Serialization specialisations
    void Serialize(std::ofstream& stream, const std::string& string);
    void Deserialize(std::ifstream& stream, std::string& string);

    // Expose-to-editor generic custom type
    template <typename T>
    bool Expose(ExposedPropertyData& propertyData, const std::string& propertyName, T& data)
    {
        propertyData.propertyName = propertyName;

        size_t cachedSize = propertyData.propertyChain.size();
        data << propertyData;
        return propertyData.propertyChain.size() > cachedSize;
    }
    // Expose-to-editor specialisations
    bool Expose(ExposedPropertyData& propertyData, const char* propertyName, int& data);
    bool Expose(ExposedPropertyData& propertyData, const char* propertyName, float& data);
    bool Expose(ExposedPropertyData& propertyData, const char* propertyName, double& data);
    bool Expose(ExposedPropertyData& propertyData, const char* propertyName, bool& data);
    bool Expose(ExposedPropertyData& propertyData, const char* propertyName, std::string& data);

    // DeletePropertyData unhandled custom type
    template <typename T>
    void DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step, const T& arg)
    {
        PropertyManipulator deleter;
        deleter.propertyChain = &propertyChain;
        deleter.step = step;
        deleter.allocatedValue = &allocatedData;

        arg >> deleter;
    }
    // DeletePropertyData handled types
    void DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step, const int& arg);
    void DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step, const float& arg);
    void DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step, const double& arg);
    void DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step, const bool& arg);
    void DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step, const std::string& arg);
    template <typename T>
    void DeletePropertyDataInternal(const void*& allocatedData)
    {
        const T* typedData = static_cast<const T*>(allocatedData);
        delete typedData;
        allocatedData = nullptr;
    }

    // SetProperty unhandled custom type
    template <typename T>
    void SetProperty(const void* newValue, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step, T& arg)
    {
        PropertyManipulator setter;
        setter.propertyChain = &propertyChain;
        setter.step = step;
        setter.value = newValue;
        setter.outPropertyData = outPropertyData;

        arg << setter;
    }
    // SetProperty handled types
    void SetProperty(const void* newValue, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step, int& arg);
    void SetProperty(const void* newValue, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step, float& arg);
    void SetProperty(const void* newValue, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step, double& arg);
    void SetProperty(const void* newValue, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step, bool& arg);
    void SetProperty(const void* newValue, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step, std::string& arg);
    template <typename T>
    void SetPropertyInternal(const void* newValue, T& property, ModifiedPropertyData* outPropertyData)
    {
        if (outPropertyData)
        {
            outPropertyData->SetOldValue(property);
        }
        const T* newValueAsType = static_cast<const T*>(newValue);
        ASSERT(newValueAsType != nullptr);
        property = *newValueAsType;
    }


    bool ExposeCategory(const char* category);
    void PopCategory();

    std::string GetPropertyName(const char* propertyNames, int propertyIndex);

    template <typename T>
    void ExposeInEditor(ExposedPropertyData& propertyData, const char* propertyNames, int propertyId, T& data)
    {
        const std::string propertyName = GetPropertyName(propertyNames, propertyId);
        if (Expose(propertyData, propertyName.c_str(), data))
        {
            propertyData.propertyChain.emplace_back(propertyId);
        }
    }
}
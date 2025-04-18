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
#define EXPOSE_PROPERTIES(outPropertyChain, ...)\
int i{0}; Serializer::ForwardArgsToSerializers([&](auto& arg) { Serializer::ExposeInEditor(outPropertyChain, #__VA_ARGS__, i++, arg); }, __VA_ARGS__);
#define RETURN_PROPERTY(propertyChain, step, propertyId, ...)\
int i{0}; const void* property{nullptr}; Serializer::ForwardArgsToSerializers([&](auto& arg) { if(i++ == propertyId) { property = Serializer::GetProperty(propertyChain, step, arg); }}, __VA_ARGS__); return property;
#define SET_PROPERTY(newValue, propertyChain, step, propertyId, ...)\
int i{0}; Serializer::ForwardArgsToSerializers([&](auto& arg) { if(i++ == propertyId) { Serializer::SetProperty(newValue, propertyChain, step, arg); }}, __VA_ARGS__);

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
virtual void ExposeToEditor(std::vector<int>& outPropertyChain)\
{\
    if(Serializer::ExposeCategory(#Class))\
    {\
        EXPOSE_PROPERTIES(outPropertyChain, __VA_ARGS__);\
        Serializer::PopCategory();\
    }\
}\
virtual const void* GetProperty(const std::vector<int>& propertyChain, int step) const\
{\
	const int propertyId = propertyChain[propertyChain.size() - step];\
    ASSERT(propertyId != -1);\
	RETURN_PROPERTY(propertyChain, step + 1, propertyId, __VA_ARGS__);\
}\
virtual void SetProperty(const void* newValue, const std::vector<int>& propertyChain, int step)\
{\
	const int propertyId = propertyChain[propertyChain.size() - step];\
    ASSERT(propertyId != -1);\
	SET_PROPERTY(newValue, propertyChain, step + 1, propertyId, __VA_ARGS__);\
}\
Class& operator<<(EditorExposer& editor)\
{\
    ExposeToEditor(*editor.outPropertyChain);\
    return *this;\
}\
const Class& operator>>(EditorManipulator& extractor) const\
{\
    extractor.value = GetProperty(*extractor.propertyChain, extractor.step);\
    return *this;\
}\
Class& operator<<(EditorManipulator& inserter)\
{\
    SetProperty(inserter.value, *inserter.propertyChain, inserter.step);\
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
virtual void ExposeToEditor(std::vector<int>& outPropertyChain) override\
{\
    if(Serializer::ExposeCategory(#Class))\
    {\
        EXPOSE_PROPERTIES(outPropertyChain, __VA_ARGS__);\
        const size_t cachedSize = outPropertyChain.size();\
        BaseClass::ExposeToEditor(outPropertyChain);\
        if(outPropertyChain.size() > cachedSize)\
        {\
            outPropertyChain.emplace_back(-1);\
        }\
        Serializer::PopCategory();\
    }\
}\
virtual const void* GetProperty(const std::vector<int>& propertyChain, int step) const override\
{\
	const int propertyId = propertyChain[propertyChain.size() - step];\
	if(propertyId == -1)\
	{\
		return BaseClass::GetProperty(propertyChain, step + 1);\
	}\
	RETURN_PROPERTY(propertyChain, step + 1, propertyId, __VA_ARGS__);\
}\
virtual void SetProperty(const void* newValue, const std::vector<int>& propertyChain, int step) override\
{\
	const int propertyId = propertyChain[propertyChain.size() - step];\
    if(propertyId == -1)\
	{\
		BaseClass::SetProperty(newValue, propertyChain, step + 1);\
        return;\
	}\
	SET_PROPERTY(newValue, propertyChain, step + 1, propertyId, __VA_ARGS__);\
}\
Class& operator<<(EditorExposer& editor)\
{\
    ExposeToEditor(*editor.outPropertyChain);\
    return *this;\
}\
const Class& operator>>(EditorManipulator& extractor) const\
{\
    extractor.value = GetProperty(*extractor.propertyChain, extractor.step);\
    return *this;\
}\
Class& operator<<(EditorManipulator& inserter)\
{\
    SetProperty(inserter.value, *inserter.propertyChain, inserter.step);\
    return *this;\
}\

struct GameObjectPtrBase;

// Used to generically expose variables to ImGui panels in Level Editor using '<<' operator
struct EditorExposer
{
    std::string propertyName;
    std::vector<int>* outPropertyChain{nullptr};
    bool modified{false};
};

// Used to generically get/set property values in Level Editor using '<<' and '>>' operators
struct EditorManipulator
{
    const std::vector<int>* propertyChain{nullptr};
    int step{0};
    const void* value{nullptr};
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
    bool Expose(std::vector<int>& outPropertyChain, const std::string& propertyName, T& data)
    {
        EditorExposer editor{};
        editor.propertyName = propertyName;
        editor.outPropertyChain = &outPropertyChain;
        editor.modified = false;

        const size_t cachedSize{outPropertyChain.size()};
        data << editor;
        return editor.modified || outPropertyChain.size() > cachedSize;
    }
    // Expose-to-editor specialisations
    bool Expose(std::vector<int>& outPropertyChain, const char* propertyName, int& data);
    bool Expose(std::vector<int>& outPropertyChain, const char* propertyName, float& data);
    bool Expose(std::vector<int>& outPropertyChain, const char* propertyName, double& data);
    bool Expose(std::vector<int>& outPropertyChain, const char* propertyName, bool& data);
    bool Expose(std::vector<int>& outPropertyChain, const char* propertyName, std::string& data);

    // GetProperty generic custom type
    template <typename T>
    const void* GetProperty(const std::vector<int>& propertyChain, int step, const T& arg)
    {
        EditorManipulator extractor{};
        extractor.propertyChain = &propertyChain;
        extractor.step = step;
        extractor.value = nullptr;

        arg >> extractor;

        return extractor.value;
    }
    // GetProperty specialisation
    const void* GetProperty(const std::vector<int>& propertyChain, int step, const int& arg);
    const void* GetProperty(const std::vector<int>& propertyChain, int step, const float& arg);
    const void* GetProperty(const std::vector<int>& propertyChain, int step, const double& arg);
    const void* GetProperty(const std::vector<int>& propertyChain, int step, const bool& arg);
    const void* GetProperty(const std::vector<int>& propertyChain, int step, const std::string& arg);

    // SetProperty generic custom type
    template <typename T>
    void SetProperty(const void* newValue, const std::vector<int>& propertyChain, int step, T& arg)
    {
        EditorManipulator inserter{0};
        inserter.propertyChain = &propertyChain;
        inserter.step = step;
        inserter.value = newValue;

        arg << inserter;
    }
    // SetProperty specialisation
    void SetProperty(const void* newValue, const std::vector<int>& propertyChain, int step, int& arg);
    void SetProperty(const void* newValue, const std::vector<int>& propertyChain, int step, float& arg);
    void SetProperty(const void* newValue, const std::vector<int>& propertyChain, int step, double& arg);
    void SetProperty(const void* newValue, const std::vector<int>& propertyChain, int step, bool& arg);
    void SetProperty(const void* newValue, const std::vector<int>& propertyChain, int step, std::string& arg);


    bool ExposeCategory(const char* category);
    void PopCategory();

    std::string GetPropertyName(const char* propertyNames, int propertyIndex);

    template <typename T>
    void ExposeInEditor(std::vector<int>& outPropertyChain, const char* propertyNames, int propertyId, T& data)
    {
        const std::string propertyName = GetPropertyName(propertyNames, propertyId);
        if (Expose(outPropertyChain, propertyName.c_str(), data))
        {
            outPropertyChain.emplace_back(propertyId);
        }
    }
}
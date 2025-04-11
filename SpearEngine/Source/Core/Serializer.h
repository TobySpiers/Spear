#pragma once

#include <iostream>
#include <fstream>

// Macro to simplify needing lambda functions to correctly resolve individual overloads
#define SERIALIZE(outStream, ...) Serializer::ForwardArgsToSerializers([&](auto& arg) { Serializer::Serialize(outStream, arg); }, __VA_ARGS__);
#define DESERIALIZE(inStream, ...) Serializer::ForwardArgsToSerializers([&](auto& arg) { Serializer::Deserialize(inStream, arg); }, __VA_ARGS__);

// Macro to simplify exposing serialized variables in the editor
#define EXPOSE(...) int i{0}; Serializer::ForwardArgsToSerializers([&](auto& arg) { Serializer::ExposeInEditor(#__VA_ARGS__, i++, arg); }, __VA_ARGS__);

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
virtual void ExposeToEditor()\
{\
    if(Serializer::ExposeCategory(#Class))\
    {\
        EXPOSE(__VA_ARGS__);\
        Serializer::PopCategory();\
    }\
}\
Class& operator<<(const EditorVariable& editor)\
{\
    ExposeToEditor();\
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
virtual void ExposeToEditor() override\
{\
    if(Serializer::ExposeCategory(#Class))\
    {\
        EXPOSE(__VA_ARGS__);\
        BaseClass::ExposeToEditor();\
        Serializer::PopCategory();\
    }\
}\
Class& operator<<(const EditorVariable& editor)\
{\
    ExposeToEditor();\
    return *this; \
}\

struct GameObjectPtrBase;

// For '<<' operator specialization used to expose ImGui accessors
struct EditorVariable
{
    const char* propertyName{nullptr};
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
    void Expose(const char* propertyName, T& data)
    {
        static EditorVariable editor{};
        editor.propertyName = propertyName;
        data << editor;
    }

    bool ExposeCategory(const char* category);
    void PopCategory();

    // Expose-to-editor specialisations
    std::string GetPropertyName(const char* propertyNames, int propertyIndex);
    void Expose(const char* propertyName, int& data);
    void Expose(const char* propertyName, float& data);
    void Expose(const char* propertyName, bool& data);
    void Expose(const char* propertyName, std::string& data);

    template <typename T>
    void ExposeInEditor(const char* propertyNames, int propertyId, T& data)
    {
        Expose(GetPropertyName(propertyNames, propertyId).c_str(), data);
    }
}
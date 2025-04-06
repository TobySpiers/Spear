#pragma once

#include <iostream>
#include <fstream>

// Macro to simplify needing lambda functions to correctly resolve individual overloads
#define SERIALIZE(outStream, ...) Serializer::ForwardArgsToSerializers([&](auto& arg) { Serializer::Serialize(outStream, arg); }, __VA_ARGS__);
#define DESERIALIZE(inStream, ...) Serializer::ForwardArgsToSerializers([&](auto& arg) { Serializer::Deserialize(inStream, arg); }, __VA_ARGS__);

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
}

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
}

struct GameObjectPtrBase;

namespace Serializer
{
    // Variadic template function using fold expression to forward each argument to appropriate overload function
    template<typename Func, typename... Args>
    void ForwardArgsToSerializers(Func func, Args&&... args)
    {
        (func(std::forward<Args>(args)), ...);
    }

    // Serialization generic functions
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

    void Serialize(std::ofstream& stream, const std::string& string);
    void Deserialize(std::ifstream& stream, std::string& string);
}
#include "Serializer.h"
#include "GameObject.h"

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
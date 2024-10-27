#pragma once

// GameObject class definition macro: must be private inside class definition
#define GAMEOBJECT_CLASS(classname)								\
virtual void Serialize(std::ofstream& os) const override\
{\
	SerializeInternal(*this, os);\
}\
static void Deserialize(std::ifstream& is)\
{\
	DeserializeInternal<classname>(is);\
}\
static bool s_factoryRegistered;

// GameObject class registration macro: must be placed in class .cpp file
#define GAMEOBJECT_REGISTER(classname)	\
bool classname::s_factoryRegistered = GameObject::RegisterClassType(typeid(classname).hash_code(), Deserialize);
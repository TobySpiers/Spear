#pragma once
#include "Core/Serializer.h"

// GameObject class definition macro: must be placed inside header
#define GAMEOBJECT_SERIALISABLE(classname, ...)								\
virtual void Serialize(std::ofstream& os) const override\
{\
	os << typeid(classname).hash_code() << std::endl;\
    SERIALIZE(os, __VA_ARGS__)\
}\
void Deserialize(std::ifstream& is)\
{\
	DESERIALIZE(is, __VA_ARGS__)\
}\
static GameObject* CreateAndDeserialise(std::ifstream& is)\
{\
	classname* obj = new classname;\
	obj->Deserialize(is);\
	return obj;\
}\
static GameObject* CreateNew()\
{\
	return GameObject::Create<classname>();\
}\
virtual const char* GetClassName() override\
{\
	return #classname;\
}\
static bool s_factoryRegistered;\

// GameObject factory registration macro: must be placed in class .cpp file
#define GAMEOBJECT_REGISTER(classname)	\
bool classname::s_factoryRegistered = GameObject::RegisterFactoryFunctionsForClass(#classname, typeid(classname).hash_code(), CreateAndDeserialise, CreateNew);
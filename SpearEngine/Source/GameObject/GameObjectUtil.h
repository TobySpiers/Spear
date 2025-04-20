#pragma once
#include "Core/Serializer.h"

// GameObject class definition macro: must be placed inside header
#define GAMEOBJECT_SERIALISABLE(classname, baseclass, ...)\
private:\
typedef baseclass Super;\
virtual void Serialize(std::ofstream& os) const override\
{\
	os << typeid(classname).hash_code() << std::endl;\
	Serialize_Internal(os);\
}\
virtual void Serialize_Internal(std::ofstream& os) const override\
{\
	Super::Serialize_Internal(os);\
    SERIALIZE(os, __VA_ARGS__)\
}\
virtual void Deserialize(std::ifstream& is) override\
{\
	Super::Deserialize(is);\
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
virtual const char* GetClassName() const override\
{\
	return #classname;\
}\
virtual void PopulateEditorPanel(ExposedPropertyData& propertyData)\
{\
	const size_t cachedSize = propertyData.propertyChain.size();\
	Super::PopulateEditorPanel(propertyData);\
	if(propertyData.propertyChain.size() > cachedSize)\
	{\
		propertyData.propertyChain.emplace_back(-1);\
	}\
	EXPOSE_PROPERTIES(propertyData, __VA_ARGS__);\
}\
virtual void SetProperty(const void* value, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData = nullptr, int step = 1) override\
{\
	const int propertyId = propertyChain[propertyChain.size() - step];\
	if(propertyId == -1)\
	{\
		Super::SetProperty(value, propertyChain, outPropertyData, step + 1);\
		return;\
	}\
	SET_PROPERTY(value, propertyChain, outPropertyData, step + 1, propertyId, __VA_ARGS__);\
}\
virtual void DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step = 1) const override\
{\
	const int propertyId = propertyChain[propertyChain.size() - step];\
	if(propertyId == -1)\
	{\
		Super::DeletePropertyData(allocatedData, propertyChain, step + 1);\
		return;\
	}\
	DELETE_PROPERTY_DATA(allocatedData, propertyChain, step + 1, propertyId, __VA_ARGS__);\
}\
static bool s_factoryRegistered;\

// GameObject factory registration macro: must be placed in class .cpp file
#define GAMEOBJECT_REGISTER(classname)	\
bool classname::s_factoryRegistered = GameObject::RegisterFactoryFunctionsForClass(#classname, typeid(classname).hash_code(), CreateAndDeserialise, CreateNew);
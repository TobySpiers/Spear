#pragma once
#include "GameObject/GameObjectPtr.h"
#include "Core/Serializer.h"

class GameObjectComponent
{
public:
    virtual void Serialize(std::ofstream& stream) const {};
    
    virtual void Deserialize(std::ifstream& stream) {};
    
    virtual void PostDeserialize() {};
    
    friend std::ofstream& operator<<(std::ofstream& stream, const GameObjectComponent& obj) 
    {
        obj.Serialize(stream); return stream;
    }
    
    friend std::ifstream& operator>>(std::ifstream& stream, GameObjectComponent& obj) 
    {
        obj.Deserialize(stream); return stream;
    }
    
    virtual void ExposeToEditor(ExposedPropertyData& propertyData) {};
    
    virtual void SetProperty(const void* newValue, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData, int step) {};
    
    virtual void DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step) const {};
    
    GameObjectComponent& operator<<(ExposedPropertyData& editor) 
    {
        ExposeToEditor(editor); return *this;
    }
    
    GameObjectComponent& operator<<(PropertyManipulator& inserter) 
    {
        SetProperty(inserter.value, *inserter.propertyChain, inserter.outPropertyData, inserter.step); return *this;
    }
    
    const GameObjectComponent& operator>>(PropertyManipulator& deleter) const 
    {
        DeletePropertyData(*deleter.allocatedValue, *deleter.propertyChain, deleter.step); return *this;
    }

	virtual ~GameObjectComponent() = default;

	void Initialise(GameObject* owner);
	GameObject* GetOwner() const;

private:
	GameObjectPtr<GameObject> m_owner;
};


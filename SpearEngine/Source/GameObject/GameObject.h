#pragma once
#include "Core/Core.h"
#include "GameObjectPtr.h"
#include "GameObjectUtil.h"
#include <fstream>
#include <new>
#include <unordered_set>
#include <cstdint>

// Note: built-in serialization does not currently support pointers. If needed, this must be handled manually via OnSerialize overrides.

enum class eGameObjectTickState
{
	TickEnabled,
	TickPendingDisable,
	TickDisabled,
};

// Base class for any type of GameObject. Derived classes must use included GAMEOBJECT_SERIALIZABLE and GAMEOBJECT_REGISTER macros.
// Constructors/destructors CAN be used to configure initial data - this will not interfere with serialization/deserialization.
// Constructors/destructors MUST NOT be used to create/destroy GameObjects - this should be performed inside OnCreated/OnDestroy functions.
class GameObject
{
	friend class GameObjectPtrBase;
	template <typename T> friend class GameObjectPtr;

public:
	template<typename T> static T* Create();

	// Implementable ---

	// Order of operations when deserializing: DefaultConstructor -> Variable Assignment From File -> OnDeserialize -> OnCreated
	virtual void OnCreated() {};
	virtual void OnTick(float deltaTime) {};
	virtual void OnDraw() const {};
	virtual void OnDestroy() {};
	virtual void OnPreSerialize() {};
	virtual void OnPostSerialize() {};
	virtual void OnDeserialize() {};

	// Native ---

	static void GlobalSerialize(const char* filename);
	static void GlobalDeserialize(const char* filename);

	static void GlobalTick(float deltaTime);
	static void GlobalDraw();
	static void GlobalDestroy();


	void SetTickEnabled(bool bShouldTick);
	bool IsTickEnabled() const;

	void SetDrawEnabled(bool bShouldDraw);
	bool IsDrawEnabled() const;

	void Destroy();
	bool IsPendingDestroy() const;
	bool IsSafeToDestroy() const;

protected:
	
	GameObject() {};
	virtual ~GameObject();

	virtual void Serialize(std::ofstream& os) const = 0;
	static bool bDeserializing;

	using DeserializeFuncPtr = GameObject*(*)(std::ifstream& is);
	static bool RegisterClassDeserializer(size_t hashcode, DeserializeFuncPtr func);

	int internalId{ -1 };

private:
	static void RegisterTickingObject(GameObject* obj);
	static void RegisterDrawingObject(GameObject* obj);

	void RegisterPtr(GameObjectPtrBase* ptr);
	void DeregisterPtr(GameObjectPtrBase* ptr);
	std::unordered_set<GameObjectPtrBase*> trackedPtrs;

	static std::unordered_map<size_t, DeserializeFuncPtr>* ObjectDeserializers();
	static std::unordered_map<size_t, DeserializeFuncPtr>* s_objectDeserializers;
	static std::vector<GameObject*> s_allocatedObjects;
	static std::vector<GameObject*> s_tickList;
	static std::vector<GameObject*> s_drawList;
	static std::vector<GameObject*> s_destroyList;

	eGameObjectTickState tickState{ eGameObjectTickState::TickDisabled };
	eGameObjectTickState drawState{ eGameObjectTickState::TickDisabled };
	bool bPendingDestroy{ false };
};

template<typename T>
T* GameObject::Create()
{
	static_assert(std::is_base_of<GameObject, T>::value, "Requested class must derive from GameObject!");
	ASSERT(!bDeserializing);
	T* obj = new T();
	obj->internalId = s_allocatedObjects.size();
	s_allocatedObjects.push_back(obj);
	obj->OnCreated();
	return obj;
}
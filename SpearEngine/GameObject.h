#pragma once
#include "Core.h"
#include "GameObjectUtil.h"
#include <fstream>

// CAUTION: Linker>UseLibraryDependencyInputs must be TRUE for any project using this class. Object self-registration may otherwise break.
// Note: built-in serialization does not currently support pointers. If needed, this must be handled manually via OnSerialize overrides.

enum class eGameObjectTickState
{
	TickEnabled,
	TickPendingDisable,
	TickDisabled,
};

// Base class for any type of GameObject. Must use included GAMEOBJECT_CLASS and GAMEOBJECT_REGISTER macros to enable serialization.
// Prefer using OnCreated/OnDestroy where possible. Constructors/destructors are both called during deserialization which can cause issues.
class GameObject
{
public:
	template<typename T> static T* Create();

	// Implementable ---

	// Fires when first created (for saved objects, this will trigger post-deserialization)
	virtual void OnCreated() {};
	virtual void OnTick(float deltaTime) {};
	virtual void OnDraw() const {};
	virtual void OnDestroy() {};
	virtual void OnSerialize() {};

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
	virtual void Serialize(std::ofstream& os) const = 0;
	template<typename T> static void SerializeInternal(const T& data, std::ofstream& os);
	template<typename T> static void DeserializeInternal(std::ifstream& is);

	using DeserializeFuncPtr = void(*)(std::ifstream& is);
	static bool RegisterClassType(size_t hashcode, DeserializeFuncPtr func);

	int internalId{ -1 };

private:
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
	T* obj = new T();
	obj->internalId = s_allocatedObjects.size();
	s_allocatedObjects.push_back(obj);
	obj->OnCreated();
	return obj;
}

template<typename T>
void GameObject::SerializeInternal(const T& data, std::ofstream& os)
{
	static_assert(std::is_base_of<GameObject, T>::value, "Requested class must derive from GameObject!");
	os << typeid(T).hash_code() << std::endl;
	os.write(reinterpret_cast<const char*>(&data), sizeof(T));
}

template<typename T>
void GameObject::DeserializeInternal(std::ifstream& is)
{
	static_assert(std::is_base_of<GameObject, T>::value, "Requested class must derive from GameObject!");
	T* tempData = new T();
	is.read(reinterpret_cast<char*>(tempData), sizeof(T));
	if (tempData->IsPendingDestroy())
	{
		delete tempData;
		return;
	}
	T* obj = new T();
	// deserializing entire objects breaks derived class vtables
	// not the most elegant workaround, but copying the deserialized object here to a fresh object avoids the issue:
	*obj = *tempData;
	delete tempData;
	obj->internalId = s_allocatedObjects.size();
	s_allocatedObjects.push_back(obj);
	if (obj->IsTickEnabled())
	{
		s_tickList.push_back(obj);
	}
	if (obj->IsDrawEnabled())
	{
		s_drawList.push_back(obj);
	}
	obj->OnCreated();
}
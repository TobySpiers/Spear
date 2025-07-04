#pragma once
#include "Core/Core.h"
#include "GameObjectPtr.h"
#include "GameObjectUtil.h"
#include <fstream>
#include <new>
#include <unordered_set>
#include <cstdint>

// Note: built-in serialization does not currently support pointers. If needed, this must be handled manually via OnSerialize overrides.

class ExposedPropertyData;
class ModifiedPropertyData;

enum class eGameObjectTickState
{
	TickEnabled,
	TickPendingDisable,
	TickDisabled,
};

enum class eGameObjectLifeState
{
	Active, // object is alive and usable
	EditorDestroy, // (editor-only): object is psuedo destroyed (exists but hidden/inactive), kept in memory for simple undo/redo
	PendingDestroy, // object is pending destroy and will no longer exist
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

	virtual void OnCreated() {};
	virtual void OnTick(float deltaTime) {};
	virtual void OnDraw() const {};
	virtual void OnDestroy() {};

	// Order of operations when deserializing: DefaultConstructor -> Variable Assignment From File -> OnDeserialize -> OnCreated
	virtual void OnDeserialize() {};
	virtual void OnPreSerialize() {};
	virtual void OnPostSerialize() {};

	// Editor
	virtual void PopulateEditorPanel(ExposedPropertyData& propertyData);
	virtual void SetProperty(const void* value, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData = nullptr, int step = 1);
	virtual void DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step = 1) const;
	virtual void DrawInEditor(const Vector3f& position, float zoom) const;
	virtual void DrawInEditorHovered(const Vector3f& position, float zoom) const;
	virtual void DrawInEditorSelected(const Vector3f& position, float zoom) const;
	void SetDestroyedInEditor(bool bDestroyed = true);
	bool IsDestroyedInEditor() const;

	// Native ---

	virtual const char* GetClassName() const;

	static void GlobalSerialize(const char* filename);
	static void GlobalSerialize(std::ofstream& file);
	static void GlobalDeserialize(const char* filename);
	static void GlobalDeserialize(std::ifstream& file);

	static void GlobalTick(float deltaTime);
	static void GlobalDraw();
	static void GlobalDestroy();

	void SetTickEnabled(bool bShouldTick);
	bool IsTickEnabled() const;

	void SetDrawEnabled(bool bShouldDraw);
	bool IsDrawEnabled() const;

	// Returns true if Object is not PendingDestroy and not marked DestroyedInEditor.
	bool IsValid() const;

	// Marks object as PendingDestroy. Destruction may not occur for several GlobalTicks. If immediate destruction is needed, call FlushPendingDestroys() afterwards.
	void Destroy();
	bool IsPendingDestroy() const;
	bool IsSafeToDestroy() const;

	// Forces immediate destruction for all PendingDestroy objects.
	static void FlushPendingDestroys();

	const Vector3f& GetPosition() const { return m_position; }
	void SetPosition(const Vector3f& position) { m_position = position; }

private:
	Vector3f m_position{0.f, 0.f, 0.f};

public:
	using ConstructorFuncPtr = GameObject*(*)();
	static std::vector<std::pair<const char*, ConstructorFuncPtr>>* ObjectConstructors();
	static const std::vector<GameObject*>& GetAllObjects() { return s_allocatedObjects; }

protected:
	
	GameObject() {};
	virtual ~GameObject();

	virtual void Serialize(std::ofstream& os) const = 0;
	virtual void Serialize_Internal(std::ofstream& os) const;
	virtual void Deserialize(std::ifstream& is);

	using DeserializeFuncPtr = GameObject*(*)(std::ifstream& is);
	static bool RegisterFactoryFunctionsForClass(const char* className, size_t hashcode, DeserializeFuncPtr deserializeFunc, ConstructorFuncPtr constructorFunc);

	int internalId{ -1 };

private:
	static void DisableTick_Internal(GameObject* obj, int ticklistIndex);
	static void DisableDraw_Internal(GameObject* obj, int drawlistIndex);
	static void DestroyObjects_Internal();

	static void RegisterTickingObject(GameObject* obj);
	static void RegisterDrawingObject(GameObject* obj);

	void RegisterPtr(GameObjectPtrBase* ptr);
	void DeregisterPtr(GameObjectPtrBase* ptr);
	std::unordered_set<GameObjectPtrBase*> trackedPtrs;

	static std::unordered_map<size_t, DeserializeFuncPtr>* ObjectDeserializers();
	static std::unordered_map<size_t, DeserializeFuncPtr>* s_objectDeserializers;
	static std::vector<std::pair<const char*, ConstructorFuncPtr>>* s_objectConstructors;

	static std::vector<GameObject*> s_allocatedObjects;
	static std::vector<GameObject*> s_tickList;
	static std::vector<GameObject*> s_drawList;
	static std::vector<GameObject*> s_destroyList;

	eGameObjectTickState tickState{ eGameObjectTickState::TickDisabled };
	eGameObjectTickState drawState{ eGameObjectTickState::TickDisabled };
	eGameObjectLifeState lifeState{ eGameObjectLifeState::Active };
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
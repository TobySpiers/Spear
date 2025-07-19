#pragma once
#include "Core/Core.h"
#include "GameObjectPtr.h"
#include "GameObjectUtil.h"
#include "Components/GameObjectComponent.h"
#include <fstream>
#include <new>
#include <unordered_set>
#include <cstdint>
#include <memory>

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
	Active, // object is alive and usable (default)
	EditorDestroy, // (editor-only): object is psuedo destroyed (exists but hidden/inactive), kept in memory for simple undo/redo
	PendingDestroy, // object is pending destroy and will no longer exist in a frame or two
};

enum class eGameObjectEditorViewMode
{
	Default,
	Hovered,
	Selected
};

// Base class for any type of GameObject. Derived classes must use included GAMEOBJECT_SERIALIZABLE and GAMEOBJECT_REGISTER macros.
// Constructors/destructors CAN be used to configure initial data - this will not interfere with serialization/deserialization but constructor values may be overidden by deserialization.
// Constructors/destructors MUST NOT be used to create/destroy GameObjects - this should be performed inside OnCreated/OnDestroy functions.
class GameObject
{
	friend class GameObjectPtrBase;
	template <typename T> friend class GameObjectPtr;
	
public:
	template<typename T> static T* Create();

	// Editor functionality
	void DrawInEditor(const Vector3f& position, float zoom, float mapSpacing, bool bSelected, bool bHovered);
	virtual void PopulateEditorPanel(ExposedPropertyData& propertyData);
	virtual void SetProperty(const void* value, const std::vector<int>& propertyChain, ModifiedPropertyData* outPropertyData = nullptr, int step = 1);
	virtual void DeletePropertyData(const void*& allocatedData, const std::vector<int>& propertyChain, int step = 1) const;

	virtual float GetEditorHoverRadius(float zoom);
	void SetDestroyedInEditor(bool bDestroyed = true);
	bool IsDestroyedInEditor() const;

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

	using ConstructorFuncPtr = GameObject*(*)();
	static std::vector<std::pair<const char*, ConstructorFuncPtr>>* ObjectConstructors();
	static const std::vector<GameObject*>& GetAllObjects() { return s_allocatedObjects; }

	template<typename T, typename... Args>
	T* AddComponent(Args&&... args)
	{
		static_assert(std::is_base_of<GameObjectComponent, T>::value, "Requested class must derive from GameObjectComponent!");
		T* component = new T(std::forward<Args>(args)...);
		component->Initialise(this);
		m_components.emplace_back(component);
		return component;
	}

	template<typename T>
	T* FindComponent() const
	{
		static_assert(std::is_base_of<GameObjectComponent, T>::value, "Requested class must derive from GameObjectComponent!");
		for (std::unique_ptr<GameObjectComponent>& componentPtr : m_components)
		{
			if (T* component = dynamic_cast<T*>(componentPtr.get()))
			{
				return component;
			}
		}
		return nullptr;
	}

protected:
	
	GameObject() {};
	virtual ~GameObject();

	// Implementable behaviours ---

	virtual void OnCreated() {};
	virtual void OnTick(float deltaTime) {};
	virtual void OnDraw() const {};
	virtual void OnEditorDraw(const Vector3f& position, float zoom, float mapSpacing);
	virtual void OnDestroy() {};

	// Order of operations when deserializing: DefaultConstructor -> Variable Assignment From File -> OnDeserialize -> OnCreated
	virtual void OnDeserialize() {};
	virtual void OnPreSerialize() {};
	virtual void OnPostSerialize() {};

	virtual void Serialize(std::ofstream& os) const {};
	virtual void Serialize_Internal(std::ofstream& os) const;
	virtual void Deserialize(std::ifstream& is);

	using DeserializeFuncPtr = GameObject*(*)(std::ifstream& is);
	static bool RegisterFactoryFunctionsForClass(const char* className, size_t hashcode, DeserializeFuncPtr deserializeFunc, ConstructorFuncPtr constructorFunc);

	int internalId{ -1 };

private:
	void OnCreated_Internal();
	void OnTick_Internal(float deltaTime);
	void OnDraw_Internal() const;
	void OnEditorDraw_Internal(const Vector3f& position, float zoom, float mapSpacing);
	void OnDestroy_Internal();

	static void DisableTick_Internal(GameObject* obj, int ticklistIndex);
	static void DisableDraw_Internal(GameObject* obj, int drawlistIndex);
	static void DestroyObjects_Internal();

	static void RegisterTickingObject(GameObject* obj);
	static void RegisterDrawingObject(GameObject* obj);

	void RegisterPtr(GameObjectPtrBase* ptr);
	void DeregisterPtr(GameObjectPtrBase* ptr);
	std::unordered_set<GameObjectPtrBase*> trackedPtrs;

	// unique_ptr means we don't have to worry about calling delete if GameObject or Component gets destroyed
	std::vector<std::unique_ptr<GameObjectComponent>> m_components;

	eGameObjectTickState tickState{ eGameObjectTickState::TickDisabled };
	eGameObjectTickState drawState{ eGameObjectTickState::TickDisabled };
	eGameObjectLifeState lifeState{ eGameObjectLifeState::Active };

	Vector3f m_position{ 0.f, 0.f, 0.f };

	static std::unordered_map<size_t, DeserializeFuncPtr>* ObjectDeserializers();
	static std::unordered_map<size_t, DeserializeFuncPtr>* s_objectDeserializers;
	static std::vector<std::pair<const char*, ConstructorFuncPtr>>* s_objectConstructors;

	static std::vector<GameObject*> s_allocatedObjects;
	static std::vector<GameObject*> s_tickList;
	static std::vector<GameObject*> s_drawList;
	static std::vector<GameObject*> s_destroyList;

};

template<typename T>
T* GameObject::Create()
{
	static_assert(std::is_base_of<GameObject, T>::value, "Requested class must derive from GameObject!");
	T* obj = new T();
	obj->internalId = s_allocatedObjects.size();
	s_allocatedObjects.push_back(obj);
	obj->OnCreated_Internal();
	return obj;
}
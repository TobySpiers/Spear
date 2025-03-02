#include "GameObject.h"
#include <algorithm>
#include <Core/FrameProfiler.h>

// Tracking
std::vector<GameObject*> GameObject::s_allocatedObjects;
std::vector<GameObject*> GameObject::s_tickList;
std::vector<GameObject*> GameObject::s_drawList;
std::vector<GameObject*> GameObject::s_destroyList;
std::unordered_map<size_t, GameObject::DeserializeFuncPtr>* GameObject::s_objectDeserializers = nullptr;
std::vector<std::pair<const char*, GameObject::ConstructorFuncPtr>>* GameObject::s_objectConstructors = nullptr;

void GameObject::GlobalTick(float deltaTime)
{	
	START_PROFILE("GameObject Update");
	for (int i = s_tickList.size() - 1; i >= 0; i--)
	{
		GameObject* obj = s_tickList[i];
		if (!obj->IsTickEnabled())
		{
			std::iter_swap(s_tickList.begin() + i, s_tickList.end());
			s_tickList.pop_back();
			obj->tickState = eGameObjectTickState::TickDisabled;
			if (obj->IsPendingDestroy() && obj->IsSafeToDestroy())
			{
				s_destroyList.push_back(obj);
			}
		}
		else
		{
			obj->OnTick(deltaTime);
		}
	}

	while (s_destroyList.size())
	{
		GameObject* obj = s_destroyList.back();
		ASSERT(obj->IsSafeToDestroy());
		const int cachedInternalId = obj->internalId;
		obj->OnDestroy();
		for (GameObjectPtrBase* ptr : obj->trackedPtrs)
		{
			ptr->Invalidate();
		}
		std::iter_swap(s_allocatedObjects.begin() + cachedInternalId, s_allocatedObjects.end() - 1);
		GameObject* swappedObj = s_allocatedObjects[cachedInternalId];
		swappedObj->internalId = cachedInternalId;
		s_allocatedObjects.pop_back();
		delete obj;
		s_destroyList.pop_back();
	}
	END_PROFILE("GameObject Update");
}

void GameObject::GlobalDraw()
{
	START_PROFILE("GameObject Draw");
	for (int i = s_drawList.size() - 1; i >= 0; i--)
	{
		GameObject* obj = s_drawList[i];
		if (!obj->IsDrawEnabled())
		{
			std::iter_swap(s_drawList.begin() + i, s_drawList.end());
			s_drawList.pop_back();
			obj->drawState = eGameObjectTickState::TickDisabled;
			if (obj->IsPendingDestroy() && obj->IsSafeToDestroy())
			{
				s_destroyList.push_back(obj);
			}
		}
		else
		{
			obj->OnDraw();
		}
	}
	END_PROFILE("GameObject Draw");
}

void GameObject::GlobalDestroy()
{
	s_tickList.clear();
	s_drawList.clear();
	s_destroyList.clear();
	while (s_allocatedObjects.size())
	{
		GameObject* obj = s_allocatedObjects.back();
		obj->OnDestroy();
		delete obj;
		s_allocatedObjects.pop_back();
	}
}

void GameObject::RegisterTickingObject(GameObject* obj)
{
	// make sure nothing is trying to add duplicates to the ticklist
	// this check isn't very performant but will be stripped out in Release builds
	ASSERT(std::find(s_tickList.begin(), s_tickList.end(), obj) == s_tickList.end()); 
	s_tickList.push_back(obj);
}

void GameObject::RegisterDrawingObject(GameObject* obj)
{
	// make sure nothing is trying to add duplicates to the ticklist
	// this check isn't very performant but will be stripped out in Release builds
	ASSERT(std::find(s_drawList.begin(), s_drawList.end(), obj) == s_drawList.end());
	s_drawList.push_back(obj);
}

void GameObject::RegisterPtr(GameObjectPtrBase* ptr)
{
	ASSERT(ptr);
	trackedPtrs.insert(ptr);
}

void GameObject::DeregisterPtr(GameObjectPtrBase* ptr)
{
	ASSERT(ptr);
	trackedPtrs.erase(ptr);
}

const char* GameObject::GetClassName()
{
	return "UnknownObject";
}

void GameObject::GlobalSerialize(const char* filename)
{
	std::ofstream file(filename);

	// Save out num objects to expect on deserialization
	file << s_allocatedObjects.size() << std::endl;

	// Prep all GameObjects
	for (GameObject* obj : s_allocatedObjects)
	{
		obj->OnPreSerialize();
	}

	// Serialize each object
	for (const GameObject* obj : s_allocatedObjects)
	{
		obj->Serialize(file);
	}

	// Restore any GameObjects
	for (GameObject* obj : s_allocatedObjects)
	{
		obj->OnPostSerialize();
	}
}

void GameObject::GlobalDeserialize(const char* filename)
{
	std::ifstream file(filename);

	// Load in num objects to expect
	std::string objectTotalStr;
	std::getline(file, objectTotalStr);
	int objectTotal = std::stoi(objectTotalStr);

	// Deserialize each object based on registered typeHash->deserializer map
	std::string typeHashStr;
	for (int i = 0; i < objectTotal; i++)
	{
		typeHashStr = "";
		std::getline(file, typeHashStr);
		size_t typeHash = std::stoll(typeHashStr); // size_t == long long, hence std::string -> long long
		DeserializeFuncPtr deserializeType = ObjectDeserializers()->at(typeHash); // if this FAILS, try setting Linker>UseLibraryDependencyInputs to TRUE

		GameObject* obj = deserializeType(file);
		obj->internalId = s_allocatedObjects.size();
		s_allocatedObjects.push_back(obj);
		if (obj->IsTickEnabled())
		{
			RegisterTickingObject(obj);
		}
		if (obj->IsDrawEnabled())
		{
			RegisterDrawingObject(obj);
		}
		obj->OnDeserialize();
		obj->OnCreated();
	}
}

void GameObject::SetTickEnabled(bool bShouldTick)
{
	ASSERT(!IsPendingDestroy());
	
	if (bShouldTick)
	{
		if (tickState == eGameObjectTickState::TickDisabled)
		{
			RegisterTickingObject(this);
		}
		tickState = eGameObjectTickState::TickEnabled;
	}
	else
	{
		if (tickState == eGameObjectTickState::TickEnabled)
		{
			tickState = eGameObjectTickState::TickPendingDisable;
		}
	}
}
bool GameObject::IsTickEnabled() const
{
	return tickState == eGameObjectTickState::TickEnabled;
}

void GameObject::SetDrawEnabled(bool bShouldDraw)
{
	ASSERT(!IsPendingDestroy());
	
	if (bShouldDraw)
	{
		if (drawState == eGameObjectTickState::TickDisabled)
		{
			RegisterDrawingObject(this);
		}
		drawState = eGameObjectTickState::TickEnabled;
	}
	else
	{
		if (drawState == eGameObjectTickState::TickEnabled)
		{
			drawState = eGameObjectTickState::TickPendingDisable;
		}
	}
}
bool GameObject::IsDrawEnabled() const
{
	return drawState == eGameObjectTickState::TickEnabled;
}

void GameObject::Destroy()
{
	if (!bPendingDestroy)
	{
		if (IsSafeToDestroy())
		{
			s_destroyList.push_back(this);
		}
		else
		{
			SetTickEnabled(false);
			SetDrawEnabled(false);
		}
		
		bPendingDestroy = true;
	}
}
bool GameObject::IsPendingDestroy() const
{
	return bPendingDestroy;
}

bool GameObject::IsSafeToDestroy() const
{
	return tickState == eGameObjectTickState::TickDisabled && drawState == eGameObjectTickState::TickDisabled;
}

GameObject::~GameObject()
{
	// Invalidate any GameObjectPtrs referencing this object
	for (GameObjectPtrBase* ptr : trackedPtrs)
	{
		ptr->Invalidate();
	}
}

bool GameObject::RegisterFactoryFunctionsForClass(const char* className, size_t hashcode, DeserializeFuncPtr deserializeFunc, ConstructorFuncPtr constructorFunc)
{
	ObjectDeserializers()->insert({hashcode, deserializeFunc });
	ObjectConstructors()->emplace_back(std::pair<const char*, ConstructorFuncPtr>(className, constructorFunc));
	return true;
}

std::unordered_map<size_t, GameObject::DeserializeFuncPtr>* GameObject::ObjectDeserializers()
{
	if (!s_objectDeserializers)
	{
		s_objectDeserializers = new std::unordered_map<size_t, DeserializeFuncPtr>;
	}
	return s_objectDeserializers;
}

std::vector<std::pair<const char*, GameObject::ConstructorFuncPtr>> *GameObject::ObjectConstructors()
{
	if (!s_objectConstructors)
	{
		s_objectConstructors = new std::vector<std::pair<const char*, GameObject::ConstructorFuncPtr>>;
	}
	return s_objectConstructors;
}

#include "GameObject.h"
#include <algorithm>

// Tracking
std::vector<GameObject*> GameObject::s_allocatedObjects;
std::vector<GameObject*> GameObject::s_tickList;
std::vector<GameObject*> GameObject::s_drawList;
std::vector<GameObject*> GameObject::s_destroyList;
std::unordered_map<size_t, GameObject::DeserializeFuncPtr>* GameObject::s_objectDeserializers = nullptr;

void GameObject::GlobalTick(float deltaTime)
{	
	for (int i = s_tickList.size() - 1; i >= 0; i--)
	{
		GameObject* obj = s_tickList[i];
		if (!obj->IsTickEnabled())
		{
			std::iter_swap(s_tickList.begin() + i, s_tickList.end());
			s_tickList.pop_back();
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
		std::iter_swap(s_allocatedObjects.begin() + cachedInternalId, s_allocatedObjects.end());
		s_allocatedObjects[cachedInternalId]->internalId = cachedInternalId;
		delete obj;
		s_destroyList.pop_back();
	}
}

void GameObject::GlobalDraw()
{
	for (int i = s_drawList.size() - 1; i >= 0; i--)
	{
		GameObject* obj = s_drawList[i];
		if (!obj->IsDrawEnabled())
		{
			std::iter_swap(s_drawList.begin() + i, s_drawList.end());
			s_drawList.pop_back();
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

void GameObject::GlobalSerialize(const char* filename)
{
	std::ofstream file(filename);
	// Save out num objects to expect on deserialization
	file << s_allocatedObjects.size() << std::endl;

	// Serialize each object
	for (const GameObject* obj : s_allocatedObjects)
	{
		obj->Serialize(file);
	}
}

void GameObject::GlobalDeserialize(const char* filename)
{
	GlobalDestroy();
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
		size_t typeHash = std::stoll(typeHashStr);
		DeserializeFuncPtr deserializeType = ObjectDeserializers()->at(typeHash);
		deserializeType(file);
	}
}

void GameObject::SetTickEnabled(bool bShouldTick)
{
	ASSERT(!IsPendingDestroy());

	if (bShouldTick != bTickEnabled)
	{
		if (bShouldTick)
		{
			s_tickList.push_back(this);
		}
		bTickEnabled = bShouldTick;
	}
}
bool GameObject::IsTickEnabled() const
{
	return bTickEnabled;
}

void GameObject::SetDrawEnabled(bool bShouldDraw)
{
	ASSERT(!IsPendingDestroy());

	if (bShouldDraw != bDrawEnabled)
	{
		if (bShouldDraw)
		{
			s_drawList.push_back(this);
		}
		bDrawEnabled = bShouldDraw;
	}
}
bool GameObject::IsDrawEnabled() const
{
	return bDrawEnabled;
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
	return !IsTickEnabled() && !IsDrawEnabled();
}

bool GameObject::RegisterClassType(size_t hashcode, DeserializeFuncPtr func)
{
	ObjectDeserializers()->insert({hashcode, func});
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

#include "GameObjectPtr.h"
#include "GameObject.h"

bool GameObjectPtrBase::IsValid() const
{
	return GetRawPtr() != nullptr;
}

void GameObjectPtrBase::Register(GameObject* obj)
{
	if (obj)
	{
		obj->RegisterPtr(this);
	}
}

void GameObjectPtrBase::InvalidateInternal(GameObject*& obj)
{
	if (obj)
	{
		obj->DeregisterPtr(this);
		obj = nullptr;
	}
}

void GameObjectPtrBase::Serialize(std::ofstream& stream) const
{
	// Store index rather than pointer. Making assumption no modifications will occur to GameObject array while serialization is in progress.
	stream << GetIndexForPtr(GetRawPtr()) << " ";
}

uintptr_t GameObjectPtrBase::GetIndexForPtr(const GameObject* ptr) const
{
	if (IsValid())
	{
		auto it = std::find(GameObject::s_allocatedObjects.begin(), GameObject::s_allocatedObjects.end(), ptr);
		ASSERT(it != GameObject::s_allocatedObjects.end());
		return std::distance(GameObject::s_allocatedObjects.begin(), it);
	}
	else
	{
		return UINTPTR_MAX;
	}
}

GameObject* GameObjectPtrBase::GetPtrFromIndex(uintptr_t index) const
{
	return GameObject::s_allocatedObjects.at(index);
}

#include "EditorAction_CreateObject.h"
#include "GameObject/GameObject.h"

EditorAction_CreateObject::EditorAction_CreateObject(int objectConstructorId, const Vector3f& position)
{
	GameObject::ConstructorFuncPtr ObjectConstructor = GameObject::ObjectConstructors()->at(objectConstructorId).second;
	m_ownedObject = ObjectConstructor();
	m_ownedObject->SetPosition(position);
}

EditorAction_CreateObject::~EditorAction_CreateObject()
{
}

void EditorAction_CreateObject::Undo(std::unordered_set<GameObject*>& outSelectedObjects)
{
	m_ownedObject->SetDestroyedInEditor();
	outSelectedObjects.erase(m_ownedObject);
}

void EditorAction_CreateObject::Redo(std::unordered_set<GameObject*>& outSelectedObjects)
{
	m_ownedObject->SetDestroyedInEditor(false);
	outSelectedObjects.clear();
	outSelectedObjects.emplace(m_ownedObject);
}

std::string EditorAction_CreateObject::ActionName()
{
	return std::string("Create Object (" + std::string(m_ownedObject->GetClassName()) + ")");
}

GameObject* EditorAction_CreateObject::GetCreatedObject()
{
	return m_ownedObject;
}

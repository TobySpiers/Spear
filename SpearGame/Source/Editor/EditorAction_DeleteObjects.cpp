#include "EditorAction_DeleteObjects.h"
#include "GameObject/GameObject.h"

EditorAction_DeleteObjects::EditorAction_DeleteObjects(const std::unordered_set<GameObject*>& objects)
	: m_ownedObjects(objects)
{
	for (GameObject* obj : objects)
	{
		obj->SetDestroyedInEditor();
	}
}

EditorAction_DeleteObjects::~EditorAction_DeleteObjects()
{
	// Could be we undid this and took another action (meaning objects are 'alive'), or could be we reached the end of undo History and were destroyed (meaning objects are 'destroyed')
	// Either way, this is the only action which can delete objects - therefore we know any 'destroyed' objects we reference were caused by us and should not be referenced by subsequent actions

	for (GameObject* obj : m_ownedObjects)
	{
		if (obj->IsDestroyedInEditor())
		{
			obj->Destroy();
		}
	}

	GameObject::FlushPendingDestroys(); // since editor does not tick GameObjects, we need to manually flush destroyed objects
}

void EditorAction_DeleteObjects::Undo(std::unordered_set<GameObject*>& outSelectedObjects)
{
	for (GameObject* obj : m_ownedObjects)
	{
		obj->SetDestroyedInEditor(false);
	}
	outSelectedObjects.clear();
	outSelectedObjects = m_ownedObjects;
}

void EditorAction_DeleteObjects::Redo(std::unordered_set<GameObject*>& outSelectedObjects)
{
	for (GameObject* obj : m_ownedObjects)
	{
		obj->SetDestroyedInEditor();
		outSelectedObjects.erase(obj);
	}
}

std::string EditorAction_DeleteObjects::ActionName()
{
	return std::string("(x" + std::to_string(m_ownedObjects.size()) + ") Delete Object(s)");
}
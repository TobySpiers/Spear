#pragma once
#include <unordered_set>
#include "EditorActionBase.h"

class GameObject;

class EditorAction_DeleteObjects : public EditorActionBase
{
public:
	EditorAction_DeleteObjects(const std::unordered_set<GameObject*>& objects);
	~EditorAction_DeleteObjects();

	virtual void Undo(std::unordered_set<GameObject*>& outSelectedObjects) override;
	virtual void Redo(std::unordered_set<GameObject*>& outSelectedObjects) override;

	virtual std::string ActionName() override;

private:
	std::unordered_set<GameObject*> m_ownedObjects;
};

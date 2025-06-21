#pragma once
#include "EditorActionBase.h"
#include <Core/MathVector.h>

class GameObject;

class EditorAction_CreateObject : public EditorActionBase
{
public:
	EditorAction_CreateObject(int objectConstructorId, const Vector3f& position);
	~EditorAction_CreateObject();

	virtual void Undo(std::unordered_set<GameObject*>& outSelectedObjects) override;
	virtual void Redo(std::unordered_set<GameObject*>& outSelectedObjects) override;

	virtual std::string ActionName() override;

	GameObject* GetCreatedObject();

private:
	GameObject* m_ownedObject{nullptr};
};

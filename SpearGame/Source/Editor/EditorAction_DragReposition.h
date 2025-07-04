#pragma once
#include "EditorActionBase.h"
#include <Core/MathVector.h>

class EditorAction_DragReposition : public EditorActionBase
{
public:
	EditorAction_DragReposition(const std::unordered_set<GameObject*>& objects, const Vector2f& delta);

	virtual void Undo(std::unordered_set<GameObject*>& outSelectedObjects) override;
	virtual void Redo(std::unordered_set<GameObject*>& outSelectedObjects) override;

	virtual std::string ActionName() override;

private:
	void ModifySecondaryObject(GameObject* obj);

	std::unordered_set<GameObject*> m_objects;
	Vector2f m_delta;
};

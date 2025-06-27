#pragma once
#include "EditorActionBase.h"
#include "Core/Serializer.h"
#include <vector>

class EditorAction_ModifyProperties : public EditorActionBase
{
public:
	EditorAction_ModifyProperties(const std::unordered_set<GameObject*>& objects);
	~EditorAction_ModifyProperties();

	void Expose();

	bool IsModificationInProgress() const;
	bool IsModificationComplete() const;

	virtual void Undo(std::unordered_set<GameObject*>& outSelectedObjects) override;
	virtual void Redo(std::unordered_set<GameObject*>& outSelectedObjects) override;

	virtual std::string ActionName() override;

private:
	void ModifySecondaryObject(GameObject* obj);

	std::unordered_set<GameObject*> m_objects;
	ExposedPropertyData primaryProperty;
	std::vector<ModifiedPropertyData> secondaryProperties;
};

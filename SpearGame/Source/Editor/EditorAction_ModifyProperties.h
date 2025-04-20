#pragma once
#include "EditorActionBase.h"
#include "Core/Serializer.h"
#include <vector>

class EditorAction_ModifyProperties : public EditorActionBase
{
public:
	EditorAction_ModifyProperties(GameObject* gameObject, size_t secondaryObjectsNum);
	~EditorAction_ModifyProperties();

	bool WasObjectModified() const;
	void ModifySecondaryObject(GameObject* obj);

	virtual void Undo() override;
	virtual void Redo() override;

	virtual std::string ActionName() override;

private:
	ExposedPropertyData primaryProperty;
	std::vector<ModifiedPropertyData> secondaryProperties;
};

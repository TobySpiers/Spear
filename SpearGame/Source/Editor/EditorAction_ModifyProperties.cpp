#include "EditorAction_ModifyProperties.h"
#include <Core/Core.h>

EditorAction_ModifyProperties::EditorAction_ModifyProperties(GameObject* gameObject, size_t secondaryObjectsNum)
    : primaryProperty(gameObject)
{
    secondaryProperties.reserve(secondaryObjectsNum);
}

EditorAction_ModifyProperties::~EditorAction_ModifyProperties()
{
    // ModifiedPropertyData instances allocate T data (before-state & after-state of modified property) on the heap - these must be manually released via CleanUp functions!
    for (ModifiedPropertyData& secondaryProperty : secondaryProperties)
    {
        secondaryProperty.CleanUp();
    }
    primaryProperty.CleanUp();
}

bool EditorAction_ModifyProperties::WasObjectModified() const
{
    return primaryProperty.WasModified();
}

void EditorAction_ModifyProperties::ModifySecondaryObject(GameObject* obj)
{
    secondaryProperties.emplace_back(obj, primaryProperty);
    obj->SetProperty(primaryProperty.GetNewValue(), primaryProperty.propertyChain, &secondaryProperties.back());
}

void EditorAction_ModifyProperties::Undo()
{
    primaryProperty.GetObject()->SetProperty(primaryProperty.GetOldValue(), primaryProperty.propertyChain);

    for (const ModifiedPropertyData& secondaryProperty : secondaryProperties)
    {
        secondaryProperty.GetObject()->SetProperty(secondaryProperty.GetOldValue(), primaryProperty.propertyChain);
    }
}

void EditorAction_ModifyProperties::Redo()
{
    primaryProperty.GetObject()->SetProperty(primaryProperty.GetNewValue(), primaryProperty.propertyChain);

    for (const ModifiedPropertyData& secondaryProperty : secondaryProperties)
    {
        secondaryProperty.GetObject()->SetProperty(primaryProperty.GetNewValue(), primaryProperty.propertyChain);
    }
}

std::string EditorAction_ModifyProperties::ActionName()
{
    return std::string("(x" + std::to_string(1 + secondaryProperties.size()) + ") Modified " + primaryProperty.modifiedPropertyName);
}

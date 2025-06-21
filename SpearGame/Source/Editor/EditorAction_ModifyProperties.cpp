#include "EditorAction_ModifyProperties.h"
#include <Core/Core.h>

EditorAction_ModifyProperties::EditorAction_ModifyProperties(std::unordered_set<GameObject*>& objects)
    : primaryProperty(*objects.begin())
{
    secondaryProperties.reserve(objects.size() - 1);
    m_objects = objects; // intentional copy
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

void EditorAction_ModifyProperties::Expose()
{
    primaryProperty.Expose();
    if (IsModificationComplete())
    {
        for (GameObject* obj : m_objects)
        {
            if (obj != *m_objects.begin())
            {
                ModifySecondaryObject(obj);
            }
        }
    }
}

bool EditorAction_ModifyProperties::IsModificationInProgress() const
{
    return primaryProperty.IsModifying();
}

bool EditorAction_ModifyProperties::IsModificationComplete() const
{
    return primaryProperty.WasModified();
}

void EditorAction_ModifyProperties::ModifySecondaryObject(GameObject* obj)
{
    secondaryProperties.emplace_back(obj, primaryProperty);
    obj->SetProperty(primaryProperty.GetNewValue(), primaryProperty.propertyChain, &secondaryProperties.back());
}

void EditorAction_ModifyProperties::Undo(std::unordered_set<GameObject*>& outSelectedObjects)
{
    primaryProperty.GetObject()->SetProperty(primaryProperty.GetOldValue(), primaryProperty.propertyChain);

    for (const ModifiedPropertyData& secondaryProperty : secondaryProperties)
    {
        secondaryProperty.GetObject()->SetProperty(secondaryProperty.GetOldValue(), primaryProperty.propertyChain);
    }

    outSelectedObjects = m_objects;
}

void EditorAction_ModifyProperties::Redo(std::unordered_set<GameObject*>& outSelectedObjects)
{
    primaryProperty.GetObject()->SetProperty(primaryProperty.GetNewValue(), primaryProperty.propertyChain);

    for (const ModifiedPropertyData& secondaryProperty : secondaryProperties)
    {
        secondaryProperty.GetObject()->SetProperty(primaryProperty.GetNewValue(), primaryProperty.propertyChain);
    }

    outSelectedObjects = m_objects;
}

std::string EditorAction_ModifyProperties::ActionName()
{
    return std::string("(x" + std::to_string(1 + secondaryProperties.size()) + ") Modified " + primaryProperty.modifiedPropertyName);
}
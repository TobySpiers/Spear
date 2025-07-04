#include "EditorAction_DragReposition.h"
#include "GameObject/GameObject.h"

EditorAction_DragReposition::EditorAction_DragReposition(const std::unordered_set<GameObject*>& objects, const Vector2f& delta)
    : m_objects(objects), m_delta(delta)
{ }

void EditorAction_DragReposition::Undo(std::unordered_set<GameObject*>& outSelectedObjects)
{
    for (GameObject* obj : m_objects)
    {
        obj->SetPosition(obj->GetPosition().XY() - m_delta);
    }

    outSelectedObjects = m_objects;
}

void EditorAction_DragReposition::Redo(std::unordered_set<GameObject*>& outSelectedObjects)
{
    for (GameObject* obj : m_objects)
    {
        obj->SetPosition(obj->GetPosition().XY() + m_delta);
    }

    outSelectedObjects = m_objects;
}

std::string EditorAction_DragReposition::ActionName()
{
    return std::string("(x" + std::to_string(m_objects.size()) + ") Dragged Position");
}
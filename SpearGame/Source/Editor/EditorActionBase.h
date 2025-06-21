#pragma once
#include <string>
#include <vector>
#include <unordered_set>

class GameObject;

class EditorActionBase
{
public:
	virtual void Undo(std::unordered_set<GameObject*>& outSelectedObjects) = 0;
	virtual void Redo(std::unordered_set<GameObject*>& outSelectedObjects) = 0;

	virtual std::string ActionName() = 0;

	virtual ~EditorActionBase(){};
};
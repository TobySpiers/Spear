#pragma once
#include <fstream>

class GameObject;

struct GameObjectPtrBase
{
	bool IsValid() const;
	void Register(GameObject* obj);
	void InvalidateInternal(GameObject*& obj);
	virtual void Invalidate() = 0;

	void Serialize(std::ofstream& stream) const;
	virtual void Deserialize(std::ifstream& istream) = 0;

protected:
	virtual GameObject* GetRawPtr() const = 0;
	uintptr_t GetIndexForPtr(const GameObject* ptr) const;
	GameObject* GetPtrFromIndex(uintptr_t index) const;
};
template <typename T>
struct GameObjectPtr : public GameObjectPtrBase
{
	GameObjectPtr()
	{
		static_assert(std::is_base_of<GameObject, T>::value, "Requested class must derive from GameObject!");
	};

	GameObjectPtr(T* inObject)
	{
		static_assert(std::is_base_of<GameObject, T>::value, "Requested class must derive from GameObject!");
		object = inObject;
		Register(object);
	}
	GameObjectPtr(const GameObjectPtr& other)
	{
		static_assert(std::is_base_of<GameObject, T>::value, "Requested class must derive from GameObject!");
		object = other.object;
		if (IsValid())
		{
			Register(object);
			object->RegisterPtr(this);
		}
	}
	GameObjectPtr& operator=(const GameObjectPtr& other)
	{
		if (this == &other)
		{
			return *this; // Self-assignment check
		}

		InvalidateInternal(object);
		object = other.object;
		Register(object);
	}
	GameObjectPtr& operator=(T* other)
	{
		if (object == other)
		{
			return *this;
		}

		InvalidateInternal(object);
		object = other;
		Register(object);
	}

	~GameObjectPtr()
	{
		InvalidateInternal(object);
	}

	virtual void Invalidate() override
	{
		InvalidateInternal(object);
	}

	virtual void Deserialize(std::ifstream& istream)
	{
		// Fixup index back into a pointer
		uintptr_t index;
		istream >> index;
		if (index != UINTPTR_MAX)
		{
			object = reinterpret_cast<T*>(GetPtrFromIndex(index));
		}
		else
		{
			object = nullptr;
		}
	}

	T* operator->() const
	{
		return object;
	}

	friend std::ofstream& operator<<(std::ofstream& stream, const GameObjectPtr& obj)
	{
		obj.Serialize(stream);
		return stream;
	}

	friend std::ifstream& operator>>(std::ifstream& stream, GameObjectPtr& obj)
	{
		obj.Deserialize(stream);
		return stream;
	}

	virtual GameObject* GetRawPtr() const
	{
		return object;
	}
private:

	T* object{ nullptr };
};
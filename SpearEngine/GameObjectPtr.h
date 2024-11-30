#pragma once
#include <fstream>

class GameObject;

struct GameObjectPtrBase
{
	bool IsValid() const;
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
		if (object)
		{
			inObject->RegisterPtr(this);
		}
	}
	GameObjectPtr(const GameObjectPtr& other)
	{
		static_assert(std::is_base_of<GameObject, T>::value, "Requested class must derive from GameObject!");
		object = other.object;
		if (IsValid())
		{
			object->RegisterPtr(this);
		}
	}
	GameObjectPtr& operator=(const GameObjectPtr& other)
	{
		if (this == &other)
		{
			return *this; // Self-assignment check
		}

		Invalidate();
		object = other.object;
		if (object)
		{
			object.RegisterPtr(this);
		}
	}
	GameObjectPtr& operator=(T* other)
	{
		if (object == other)
		{
			return *this;
		}

		Invalidate();
		object = other;
		if (object)
		{
			object.RegisterPtr(this);
		}
	}

	~GameObjectPtr()
	{
		Invalidate();
	}

	virtual void Invalidate() override
	{
		if (object)
		{
			object->DeregisterPtr(this);
			object = nullptr;
		}
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

private:
	virtual GameObject* GetRawPtr() const
	{
		return object;
	}

	T* object{ nullptr };
};
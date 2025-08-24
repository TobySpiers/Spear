#pragma once
#include "Core.h"
#include <functional>
#include <unordered_map>

template<typename... Args>
class Event
{
public:

	// Returns EventID for the registered function. MUST be manually unregistered when owning object is destroyed!
	template<typename T>
	EventID Register(T* obj, void (T::* callbackFunction)(Args...))
	{
		ASSERT(obj);

		// Wrap up the method pointer inside a lamba so we can pass it to the Register function
		return Register([obj, callbackFunction](Args... args)
		{
			(obj->*callbackFunction)(args...);
		});
	}

	// Returns EventID for the registered function. MUST be manually unregistered when owning object is destroyed!
	EventID Register(std::function<void(Args...)> callback)
	{
		m_callbackFunctions.insert({m_nextId, callback});
		return {m_nextId++};
	}

	// Unregisters a callback function using the value returned when registering.
	void Unregister(EventID eventId)
	{
		m_callbackFunctions.erase(eventId.id);
	}

	// Triggers all registered callbacks using supplied arguments.
	void Broadcast(Args... args)
	{
		for (const auto& [id, callbackFunction] : m_callbackFunctions)
		{
			callbackFunction(args...);
		}
	}

private:
	std::unordered_map<size_t, std::function<void(Args...)>> m_callbackFunctions;
	size_t m_nextId{1};
};


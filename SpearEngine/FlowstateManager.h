#pragma once

#include "Core.h"

namespace Spear
{
	class Flowstate
	{
	public:
		Flowstate(){};
		virtual ~Flowstate(){};

		// Called once when state begins
		virtual void StateEnter() = 0;

		// Update game. Return a slot id to pick new state:
		// return -1 to remain in current state.
		virtual int StateUpdate(float deltaTime) = 0;

		// Render game
		virtual void StateRender() = 0;

		// Called once when state ends
		virtual void StateExit() = 0;
	};

	class FlowstateManager
	{
		NO_COPY(FlowstateManager);

	public:
		FlowstateManager(){};
		~FlowstateManager(){};

		void RegisterState(Flowstate* pState, u32 slot);
		void DeregisterState(u32 slot);
		void SetInitialState(u32 slot);
		void Update(float deltaTime);

	private:
		std::unordered_map<u32, Flowstate*> m_registeredStates;
		Flowstate* m_pCurState{nullptr};
	};
}
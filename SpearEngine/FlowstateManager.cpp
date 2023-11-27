#include "Core.h"
#include "FlowstateManager.h"

#if _DEBUG
#include "FrameProfiler.h"
#endif

namespace Spear
{
	void FlowstateManager::RegisterState(Flowstate* pState, u32 slot)
	{
		ASSERT(pState);
		m_registeredStates.insert({slot, pState});
	}

	void FlowstateManager::DeregisterState(u32 slot)
	{
		// Never deregister the currently active state!
		ASSERT(m_registeredStates.at(slot) != m_pCurState);
		m_registeredStates.erase(slot);
	}

	void FlowstateManager::SetInitialState(u32 slot)
	{
		m_pCurState = m_registeredStates.at(slot);
		m_pCurState->StateEnter();
	}

	void FlowstateManager::Update(float deltaTime)
	{
		START_PROFILE("Flowstate Update");
		s8 nextStateId = m_pCurState->StateUpdate(deltaTime);
		Flowstate* pNextState{nullptr};
		if (nextStateId >= 0)
		{
			pNextState = m_registeredStates.at(nextStateId);
		}
		END_PROFILE("Flowstate Update");

		START_PROFILE("Flowstate Render");
		m_pCurState->StateRender();
		END_PROFILE("Flowstate Render");

		START_PROFILE("Flowstate Change");
		if (pNextState)
		{
			m_pCurState->StateExit();
			m_pCurState = pNextState;
			m_pCurState->StateEnter();
		}
		END_PROFILE("Flowstate Change");
	}
}
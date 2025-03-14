#include "Core/Core.h"
#include "FlowstateManager.h"
#include "FrameProfiler.h"

namespace Spear
{
	Flowstate* FlowstateManager::GetFlowstate(u32 slot)
	{
		if (m_registeredStates.size() > slot)
		{
			return m_registeredStates[slot];
		}
		else
		{
			return nullptr;
		}
	}

	bool FlowstateManager::IsFlowstateActive(Flowstate* flowstate)
	{
		return flowstate == m_pCurState;
	}

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
		Flowstate* pNextState{ nullptr };
		if (nextStateId >= 0)
		{
			pNextState = m_registeredStates.at(nextStateId);
		}

		if (pNextState)
		{
			// Finish the active state
			m_pCurState->StateExit();
			m_pCurState = pNextState;

			// Ensure Update always runs prior to Render
			m_pCurState->StateEnter();
			m_pCurState->StateUpdate(FLT_MIN);
		}

		END_PROFILE("Flowstate Update");
	}

	void FlowstateManager::Render()
	{
		START_PROFILE("Flowstate Render");
		m_pCurState->StateRender();
		END_PROFILE("Flowstate Render");
	}
}
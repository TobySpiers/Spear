#include "Core/Core.h"
#include "Core/ServiceLocator.h"
#include "Core/FlowstateManager.h"
#include "Flowstates/eFlowstate.h"
#include "Flowstates/FlowstateGame.h"
#include "GameState.h"

GameState& GameState::Get()
{
	Spear::Flowstate* fs = Spear::ServiceLocator::GetFlowstateManager().GetFlowstate((u32)eFlowstate::STATE_PLAY);
	return dynamic_cast<FlowstateGame*>(fs)->GetGameState();
}

GameState* GameState::GetSafe()
{
	Spear::FlowstateManager& fsm = Spear::ServiceLocator::GetFlowstateManager();
	if(Spear::Flowstate* flowstate = fsm.GetFlowstate((u32)eFlowstate::STATE_PLAY))
	{
		if (fsm.IsFlowstateActive(flowstate))
		{
			if (FlowstateGame* flowstateGame = dynamic_cast<FlowstateGame*>(flowstate))
			{
				return &flowstateGame->GetGameState();
			}
		}
	}
	return nullptr;
}
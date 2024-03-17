#include "BlockedState.h"
#include "WaitState.h"

void BlockedState::ChangeState(Zombie* zombie)
{
	zombie->SetZombieState(WaitState::GetInstance());
}

void BlockedState::Update(Zombie* zombie)
{
	if (zombie->Waiting())
	{
		zombie->ChangeState();
	}
}

EZombieState BlockedState::GetStateEnum()
{
	return EZombieState::BLOCKED;
}

#include "BiteState.h"
#include "WaitState.h"

void BiteState::ChangeState(Zombie* zombie)
{
	zombie->SetZombieState(WaitState::GetInstance());
}

void BiteState::Update(Zombie* zombie)
{
	if (zombie->Waiting())
	{
		zombie->ChangeState();
	}
}

EZombieState BiteState::GetStateEnum()
{
	return EZombieState::BITE;
}

#include "BiteState.h"
#include "WaitState.h"

void BiteState::ChangeState(Zombie* zombie)
{
	//zombie->SetZombieState(WaitState::GetInstance());
}

void BiteState::Update(Zombie* zombie)
{
}

EZombieState BiteState::GetStateEnum()
{
	return EZombieState::BITE;
}

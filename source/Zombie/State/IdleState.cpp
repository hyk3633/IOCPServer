#include "IdleState.h"
#include "ChaseState.h"

void IdleState::ChangeState(Zombie* zombie)
{
	zombie->SetZombieState(ChaseState::GetInstance());
}

void IdleState::Update(Zombie* zombie)
{
}

EZombieState IdleState::GetStateEnum()
{
	return EZombieState::IDLE;
}

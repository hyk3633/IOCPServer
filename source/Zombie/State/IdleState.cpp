#include "IdleState.h"
#include "ChaseState.h"
#include "../Zombie.h"

void IdleState::ChangeState(Zombie* zombie)
{
	zombie->SetZombieState(ChaseState::GetInstance());
}

void IdleState::Update(Zombie* zombie)
{
	if (zombie->CheckNearestPlayer())
	{
		zombie->ChangeState();
	}
}

EZombieState IdleState::GetStateEnum()
{
	return EZombieState::IDLE;
}

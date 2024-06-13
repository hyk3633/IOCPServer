#include "IdleState.h"
#include "ChaseState.h"
#include "../Zombie.h"

using std::shared_ptr;

void IdleState::ChangeState(shared_ptr<Zombie> zombie)
{
	zombie->SetZombieState(ChaseState::GetInstance());
}

void IdleState::Update(shared_ptr<Zombie> zombie)
{
	if (zombie->FindNearestPlayer())
	{
		zombie->ChangeState();
	}
}

EZombieState IdleState::GetStateEnum()
{
	return EZombieState::IDLE;
}

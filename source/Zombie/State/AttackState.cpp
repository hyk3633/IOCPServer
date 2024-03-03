#include "AttackState.h"
#include "WaitState.h"

void AttackState::ChangeState(Zombie* zombie)
{
	zombie->SetZombieState(WaitState::GetInstance());
}

void AttackState::Update(Zombie* zombie)
{
	
}

EZombieState AttackState::GetStateEnum()
{
	return EZombieState::ATTACK;
}

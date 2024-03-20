#include "AttackState.h"
#include "WaitState.h"
#include "../Zombie.h"

using std::shared_ptr;

void AttackState::ChangeState(shared_ptr<Zombie> zombie)
{
	zombie->SetZombieState(WaitState::GetInstance());
}

void AttackState::Update(shared_ptr<Zombie> zombie)
{
	
}

EZombieState AttackState::GetStateEnum()
{
	return EZombieState::ATTACK;
}

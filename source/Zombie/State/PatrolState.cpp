#include "PatrolState.h"
#include "ChaseState.h"

void PatrolState::ChangeState(Zombie* zombie)
{
	zombie->SetZombieState(ChaseState::GetInstance());
}

void PatrolState::Update(Zombie* zombie)
{
}

EZombieState PatrolState::GetStateEnum()
{
	return EZombieState::PATROL;
}

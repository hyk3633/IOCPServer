#include "WaitState.h"
#include "IdleState.h"
#include "PatrolState.h"
#include "ChaseState.h"
#include "AttackState.h"
#include "GrabState.h"

void WaitState::ChangeState(Zombie* zombie)
{
	//zombie->SetZombieState(IdleState::GetInstance());
	//zombie->SetZombieState(PatrolState::GetInstance());
	//zombie->SetZombieState(ChaseState::GetInstance());
	//zombie->SetZombieState(AttackState::GetInstance());
	//zombie->SetZombieState(GrabState::GetInstance());
}

void WaitState::Update(Zombie* zombie)
{

}

EZombieState WaitState::GetStateEnum()
{
	return EZombieState::WAIT;
}

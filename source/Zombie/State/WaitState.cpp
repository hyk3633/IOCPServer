#include "WaitState.h"
#include "IdleState.h"
#include "PatrolState.h"
#include "ChaseState.h"
#include "AttackState.h"
#include "GrabState.h"

void WaitState::ChangeState(Zombie* zombie)
{
	const float distance = zombie->GetZombieLocation().GetDistance(zombie->GetTargetLocation());
	if (distance <= 100.f)
	{
		// 플레이어가 wrestle 상태면 wait
		//zombie.SetZombieState(WaitState::GetInstance());
		
		zombie->SetZombieState(AttackState::GetInstance());

		//zombie->SetZombieState(GrabState::GetInstance());
	}
	else if (distance > 100.f && distance <= 1200.f)
	{
		zombie->SetZombieState(ChaseState::GetInstance());
	}
	else
	{
		zombie->SetZombieState(IdleState::GetInstance());
	}
}

void WaitState::Update(Zombie* zombie)
{
	if (zombie->Waiting())
	{
		zombie->ChangeState();
	}
}

EZombieState WaitState::GetStateEnum()
{
	return EZombieState::WAIT;
}

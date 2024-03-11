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
		if (zombie->GetTargetWrestleState() == EWrestleState::ABLE)
		{
			zombie->SetZombieState(GrabState::GetInstance());
		}
		else if (zombie->GetTargetWrestleState() == EWrestleState::WAITING)
		{
			zombie->SetZombieState(AttackState::GetInstance());
		}
		else
		{
			zombie->SetZombieState(WaitState::GetInstance());
		}
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

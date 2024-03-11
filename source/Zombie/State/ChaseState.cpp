#include "ChaseState.h"
#include "IdleState.h"
#include "WaitState.h"
#include "AttackState.h"
#include "GrabState.h"
#include "../Zombie.h"

void ChaseState::ChangeState(Zombie* zombie)
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
	else
	{
		zombie->SetZombieState(IdleState::GetInstance());
	}
}

void ChaseState::Update(Zombie* zombie)
{
	if (zombie && zombie->IsTargetSetted())
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
			zombie->ProcessMovement();
		}
		else
		{
			zombie->SetZombieState(IdleState::GetInstance());
		}
	}
}

EZombieState ChaseState::GetStateEnum()
{
	return EZombieState::CHASE;
}

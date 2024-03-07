#include "ChaseState.h"
#include "IdleState.h"
#include "WaitState.h"
#include "AttackState.h"
#include "GrabState.h"
#include "../Zombie.h"

void ChaseState::ChangeState(Zombie* zombie)
{
	// 플레이어가 wrestle 상태면 wait
	//zombie.SetZombieState(WaitState::GetInstance());

	// 플레이어가 wait 상태면 attack
	zombie->SetZombieState(AttackState::GetInstance());

	// 플레이어가 위 두 상태가 아니면 grab
	//zombie.SetZombieState(GrabState::GetInstance());
}

void ChaseState::Update(Zombie* zombie)
{
	if (zombie && zombie->IsTargetSetted())
	{
		const float distance = zombie->GetZombieLocation().GetDistance(zombie->GetTargetLocation());

		if (distance <= 100.f)
		{
			zombie->SetZombieState(AttackState::GetInstance());
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

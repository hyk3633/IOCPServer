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
		zombie->SetZombieState(AttackState::GetInstance());

		// �÷��̾ wrestle ���¸� wait
		//zombie.SetZombieState(WaitState::GetInstance());

		// �÷��̾ �� �� ���°� �ƴϸ� grab
		//zombie.SetZombieState(GrabState::GetInstance());
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
			zombie->SetZombieState(AttackState::GetInstance());

			// �÷��̾ wrestle ���¸� wait
			// �÷��̾ �� �� ���°� �ƴϸ� grab
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

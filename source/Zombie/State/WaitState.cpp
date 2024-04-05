#include "WaitState.h"
#include "IdleState.h"
#include "ChaseState.h"
#include "AttackState.h"
#include "GrabState.h"
#include "../Zombie.h"
#include "../../Player/Player.h"
#include "../../Structs/Vector3D.h"

using std::shared_ptr;

void WaitState::ChangeState(shared_ptr<Zombie> zombie)
{
	shared_ptr<Player> targetPlayer = zombie->GetTargetPlayer();
	if (zombie && targetPlayer)
	{
		const float distance = zombie->GetLocation().GetDistance(targetPlayer->GetLocation());
		if (distance <= 100.f)
		{
			if (targetPlayer->GetWrestleState() == EWrestleState::ABLE)
			{
				zombie->SetZombieState(GrabState::GetInstance());
			}
			else if (targetPlayer->GetWrestleState() == EWrestleState::WAITING)
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
	else
	{
		zombie->SetZombieState(IdleState::GetInstance());
	}
}

void WaitState::Update(shared_ptr<Zombie> zombie)
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

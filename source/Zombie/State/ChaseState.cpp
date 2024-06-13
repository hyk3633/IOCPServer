#include "ChaseState.h"
#include "IdleState.h"
#include "WaitState.h"
#include "AttackState.h"
#include "GrabState.h"
#include "../Zombie.h"
#include "../../Player/Player.h"
#include "../../Structs/Vector3D.h"

using std::shared_ptr;

void ChaseState::ChangeState(shared_ptr<Zombie> zombie)
{
	if (zombie->FindNearestPlayer() == false)
	{
		zombie->SetZombieState(IdleState::GetInstance());
	}
}

void ChaseState::Update(shared_ptr<Zombie> zombie)
{
	auto targetPlayer = zombie->GetTargetPlayer();
	if (targetPlayer)
	{
		const float distance = zombie->GetLocation().GetDistance(targetPlayer->GetLocation());
		if (distance <= 70.f)
		{
			if (targetPlayer->GetWrestleState() == EWrestleState::ABLE)
			{
				zombie->SetZombieState(GrabState::GetInstance());
			}
			else
			{
				zombie->SetZombieState(AttackState::GetInstance());
			}
		}
		else if (distance > 70.f && distance <= 1200.f)
		{
			zombie->ProcessMovement();
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

EZombieState ChaseState::GetStateEnum()
{
	return EZombieState::CHASE;
}

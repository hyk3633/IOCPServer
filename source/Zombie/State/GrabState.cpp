#include "GrabState.h"
#include "BiteState.h"
#include "BlockedState.h"
#include "../Zombie.h"
#include "../../Player/Player.h"

using std::shared_ptr;

void GrabState::ChangeState(shared_ptr<Zombie> zombie)
{
	shared_ptr<Player> targetPlayer = zombie->GetTargetPlayer();
	if (targetPlayer)
	{
		if (targetPlayer->GetSuccessToBlocking())
		{
			zombie->SetZombieState(BlockedState::GetInstance());
		}
		else
		{
			zombie->SetZombieState(BiteState::GetInstance());
		}
	}
}

void GrabState::Update(shared_ptr<Zombie> zombie)
{
	shared_ptr<Player> targetPlayer = zombie->GetTargetPlayer();
	if (targetPlayer && targetPlayer->GetWrestleState() == EWrestleState::ABLE)
	{
		targetPlayer->WrestleStateOn();
		
		const Vector3D newLocation = targetPlayer->GetLocation() + (targetPlayer->GetRotation().GetForwardVector() * 70.f);
		Rotator newRotation = zombie->GetRotation();
		newRotation.yaw = targetPlayer->GetRotation().yaw + 180.f;
		if (newRotation.yaw > 360.f)
			newRotation.yaw -= 360.f;

		zombie->SetLocation(newLocation);
		zombie->SetRotation(newRotation);
	}
}

EZombieState GrabState::GetStateEnum()
{
	return EZombieState::GRAB;
}

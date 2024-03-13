#include "GrabState.h"
#include "BiteState.h"
#include "BlockedState.h"
#include "../../CharacterInfo.h"

void GrabState::ChangeState(Zombie* zombie)
{
	if (zombie->IsTargetSetted())
	{
		if (zombie->GetTargetSuccessToBlock())
		{
			zombie->SetZombieState(BlockedState::GetInstance());
		}
		else
		{
			zombie->SetZombieState(BiteState::GetInstance());
		}
	}
}

void GrabState::Update(Zombie* zombie)
{
	if (zombie->IsTargetSetted() && zombie->GetTargetWrestleState() == EWrestleState::ABLE)
	{
		zombie->SetTargetWrestleState(EWrestleState::WRESTLING);

		const Vector3D newLocation = zombie->GetTargetLocation() + (zombie->GetTargetRotation().GetForwardVector() * 70.f);
		Rotator newRotation = zombie->GetZombieRotation();
		newRotation.yaw = zombie->GetTargetRotation().yaw + 180.f;
		if (newRotation.yaw > 360.f)
			newRotation.yaw -= 360.f;
		zombie->SetZombieLocation(newLocation);
		zombie->SetZombieRotation(newRotation);
	}
}

EZombieState GrabState::GetStateEnum()
{
	return EZombieState::GRAB;
}

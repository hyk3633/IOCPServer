#include "GrabState.h"
#include "BiteState.h"
#include "BlockedState.h"
#include "../../CharacterInfo.h"

void GrabState::ChangeState(Zombie* zombie)
{
	//zombie->SetZombieState(BiteState::GetInstance());
	//zombie->SetZombieState(BlockedState::GetInstance());
}

void GrabState::Update(Zombie* zombie)
{
	if (zombie->IsTargetSetted() && zombie->GetTargetWrestleState() == EWrestleState::ABLE)
	{
		zombie->SetTargetWrestleState(EWrestleState::WRESTLING);

		const Vector3D newLocation = zombie->GetTargetLocation() + (zombie->GetZombieRotation().GetForwardVector() * 70.f);
		zombie->SetZombieLocation(newLocation);
	}
}

EZombieState GrabState::GetStateEnum()
{
	return EZombieState::GRAB;
}

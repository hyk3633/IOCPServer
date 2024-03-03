#include "GrabState.h"
#include "BiteState.h"
#include "BlockedState.h"

void GrabState::ChangeState(Zombie* zombie)
{
	//zombie->SetZombieState(BiteState::GetInstance());
	//zombie->SetZombieState(BlockedState::GetInstance());
}

void GrabState::Update(Zombie* zombie)
{

}

EZombieState GrabState::GetStateEnum()
{
	return EZombieState::GRAB;
}

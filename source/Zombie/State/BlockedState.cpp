#include "BlockedState.h"
#include "WaitState.h"

void BlockedState::ChangeState(Zombie* zombie)
{
	zombie->SetZombieState(WaitState::GetInstance());
}

void BlockedState::Update(Zombie* zombie)
{

}

EZombieState BlockedState::GetStateEnum()
{
	return EZombieState::BLOCKED;
}

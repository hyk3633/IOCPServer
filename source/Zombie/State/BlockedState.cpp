#include "BlockedState.h"
#include "WaitState.h"
#include "../Zombie.h"

using std::shared_ptr;

void BlockedState::ChangeState(shared_ptr<Zombie> zombie)
{
	zombie->SetZombieState(WaitState::GetInstance());
}

void BlockedState::Update(shared_ptr<Zombie> zombie)
{
	if (zombie->Waiting())
	{
		zombie->ChangeState();
	}
}

EZombieState BlockedState::GetStateEnum()
{
	return EZombieState::BLOCKED;
}

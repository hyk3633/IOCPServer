#include "BiteState.h"
#include "WaitState.h"
#include "../Zombie.h"

using std::shared_ptr;

void BiteState::ChangeState(shared_ptr<Zombie> zombie)
{
	zombie->SetZombieState(WaitState::GetInstance());
}

void BiteState::Update(shared_ptr<Zombie> zombie)
{
	if (zombie->Waiting())
	{
		zombie->ChangeState();
	}
}

EZombieState BiteState::GetStateEnum()
{
	return EZombieState::BITE;
}

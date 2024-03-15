#include "Zombie.h"
#include "State/IdleState.h"
#include <iostream>

Zombie::Zombie() : zombieState(IdleState::GetInstance()), targetInfo(nullptr)
{
	pathManager = make_unique<PathManager>();
	pathManager->SetZombie(this);
}

void Zombie::ChangeState()
{
	zombieState->ChangeState(this);
	elapsedWaitingTime = 0.f;
}

void Zombie::Update()
{
	zombieState->Update(this);
}

void Zombie::SetZombieState(ZombieState* newState)
{
	zombieState = newState;
	zombieInfo.state = zombieState->GetStateEnum();
}

bool Zombie::IsTargetSetted()
{
	if (targetInfo) return true;
	else return false;
}

void Zombie::ProcessMovement()
{
	pathManager->ProcessMovement();
}

void Zombie::AddMovement(const Vector3D& direction, const Vector3D& dest)
{
	Vector3D& location = zombieInfo.location;
	const float maxStep = speed * interval;
	if ((dest - location).GetMagnitude() > maxStep)
	{
		location = location + (direction * speed * interval);
	}
	else
	{
		location = dest;
	}
	location.Truncate();
}

bool Zombie::Waiting()
{
	elapsedWaitingTime += 0.008f;
	if (elapsedWaitingTime >= waitingTime)
	{
		elapsedWaitingTime = 0.f;
		return true;
	}
	return false;
}

void Zombie::SetTargetWrestleState(const EWrestleState state)
{
	targetInfo->wrestleState = state;
	targetInfo->sendInfoBitMask |= (1 << static_cast<int>(PIBTS::WrestlingState));
}

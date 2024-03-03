#include "Zombie.h"
#include "State/IdleState.h"

Zombie::Zombie() : zombieState(IdleState::GetInstance()), targetInfo(nullptr)
{
	zombieInfo.state = EZombieState::IDLE;
	pathManager = make_unique<PathManager>();
	pathManager->SetZombie(this);
}

void Zombie::ChangeState()
{
	zombieState->ChangeState(this);
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

void Zombie::AddMovement(const Vector3D direction)
{
	zombieInfo.characterInfo.location.X += (direction.X * speed * 0.05f);
	zombieInfo.characterInfo.location.Y += (direction.Y * speed * 0.05f);
	zombieInfo.characterInfo.location.Z += (direction.Z * speed * 0.05f);
	// velocity
}

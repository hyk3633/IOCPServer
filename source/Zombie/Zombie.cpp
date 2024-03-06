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

void Zombie::AddMovement(const Vector3D& direction, const Vector3D& dest)
{
	Vector3D& location = zombieInfo.characterInfo.location;
	const float maxStep = speed * 0.016f;
	if ((dest - location).GetMagnitude() > maxStep)
	{
		location = location + (direction * speed * 0.016f);
	}
	else
	{
		location = dest;
	}
	location.Truncate();

	//zombieInfo.characterInfo.location.X += (direction.X * speed * 0.05f);
	//zombieInfo.characterInfo.location.Y += (direction.Y * speed * 0.05f);
	//zombieInfo.characterInfo.location.Z += (direction.Z * speed * 0.05f);
	
	//zombieInfo.characterInfo.velocityX = direction.X;
	//zombieInfo.characterInfo.velocityY = direction.Y;
	//zombieInfo.characterInfo.velocityZ = direction.Z;

	cout << zombieInfo.characterInfo.location.X << " " << zombieInfo.characterInfo.location.Y << " " << zombieInfo.characterInfo.location.Z << "\n";
}

void Zombie::SetSpeed(const Vector3D& dest)
{
	speed = GetZombieLocation().GetDistance(dest) / 1.5f;
	speed = 100.f;
}

void Zombie::SetPath(const vector<Pos>& path)
{
	zombieInfo.pathToTarget = path;
	zombieInfo.bSetPath = true;
}

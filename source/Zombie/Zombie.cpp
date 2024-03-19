#include "Zombie.h"
#include "PathManager.h"
#include "State/IdleState.h"
#include "../Structs/Vector3D.h"
#include "../Structs/Rotator.h"
#include "../Math/Math.h"
#include <iostream>

Zombie::Zombie() : zombieState(IdleState::GetInstance()), targetInfo(nullptr)
{
	pathManager = make_unique<PathManager>();
	pathManager->SetZombie(this);
}

Zombie::~Zombie()
{
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

void Zombie::AddToTargets(const int playerNumber, PlayerInfo* playerInfo)
{
	targetsMap[playerNumber] = playerInfo;
}

void Zombie::RemoveTargets(const int playerNumber)
{
	if (targetsMap.find(playerNumber) != targetsMap.end())
	{
		targetsMap.erase(playerNumber);
	}
}

void Zombie::AllZombieInfoBitOn()
{
	zombieInfo.sendInfoBitMask = (1 << static_cast<int>(ZIBT::MAX)) - 1;
}

void Zombie::SetZombieState(ZombieState* newState)
{
	zombieState = newState;
	zombieInfo.state = zombieState->GetStateEnum();
	MaskToInfoBit(ZIBT::State);
}

bool Zombie::CheckNearestPlayer()
{
	if (targetsMap.empty()) 
		return false;
	float minDist = 10000.f;
	int nearestNumber = -1;
	for (auto& kv : targetsMap)
	{
		PlayerInfo* info = kv.second;
		Vector3D toTarget = info->characterInfo.location - GetZombieLocation();
		const float angleDegree = RadiansToDegrees(Acos(DotProduct(toTarget.Normalize(), GetForwardVector())));
		if (angleDegree < 60.f)
		{
			const float dist = toTarget.GetMagnitude();
			if (dist < minDist)
			{
				minDist = dist;
				nearestNumber = kv.first;
			}
		}
	}
	if (nearestNumber == -1)
	{
		return false;
	}
	else
	{
		SetTargetNumber(nearestNumber);
		SetTarget(targetsMap[nearestNumber]);
		return true;
	}
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
	MaskToInfoBit(ZIBT::Location);
}

void Zombie::SetZombieRotation(const Rotator& rotation)
{
	zombieInfo.rotation = rotation;
	MaskToInfoBit(ZIBT::Rotation);
}

void Zombie::SetNextGrid(const Vector3D& nextLocation)
{
	zombieInfo.nextLocation = nextLocation;
	MaskToInfoBit(ZIBT::NextLocation);
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

void Zombie::SetTargetNumber(const int number)
{
	zombieInfo.targetNumber = number;
	MaskToInfoBit(ZIBT::TargetNumber);
}

void Zombie::SetTargetWrestle()
{
	targetInfo->wrestleState = EWrestleState::WRESTLING;
	wbCallback(zombieInfo.targetNumber);
}

void Zombie::RegisterBroadcastCallback(WrestlingBroadcast wb)
{
	wbCallback = wb;
}

void Zombie::MaskToInfoBit(const ZIBT bitType)
{
	zombieInfo.sendInfoBitMask |= (1 << static_cast<int>(bitType));
}

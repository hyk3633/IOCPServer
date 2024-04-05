#include "Zombie.h"
#include "PathManager.h"
#include "State/IdleState.h"
#include "../Player/Player.h"
#include "../Structs/Vector3D.h"
#include "../Structs/Rotator.h"
#include "../Math/Math.h"
#include <iostream>

using namespace std;

Zombie::Zombie(const int num) : Character(num), zombieState(IdleState::GetInstance())
{
	pathManager = make_unique<PathManager>();
	pathManager->SetZombie(shared_ptr<Zombie>(this));
}

Zombie::~Zombie()
{
}

void Zombie::ChangeState()
{
	zombieState->ChangeState(shared_from_this());
}

void Zombie::Update()
{
	if (stateEnum == EZombieState::IDLE)
	{
		zombieState->Update(shared_from_this());
	}
	else if (auto targetPlayer = targetWeakPtr.lock())
	{
		if (targetPlayer->GetIsDead())
		{
			targetWeakPtr.reset();
			ChangeState();
		}
		else
		{
			zombieState->Update(shared_from_this());
		}
	}
	else
	{
		ChangeState();
	}
}

void Zombie::AddPlayerToInRangeMap(shared_ptr<Player> player)
{
	inRangePlayerMap[player->GetNumber()] = player;
}

void Zombie::RemoveInRangePlayer(const int playerNumber)
{
	if (inRangePlayerMap.find(playerNumber) != inRangePlayerMap.end())
	{
		inRangePlayerMap.erase(playerNumber);
	}
}

void Zombie::AllZombieInfoBitOn()
{
	MaskToInfoBit(ZIBT::Location);
	MaskToInfoBit(ZIBT::Rotation);
	MaskToInfoBit(ZIBT::State);
	if (IsTargetValid())
	{
		MaskToInfoBit(ZIBT::TargetNumber);
	}
}

void Zombie::RegisterZombieDeadCallback(ZombieDeadCallback zdc)
{
	zombieDeadCb = zdc;
}

void Zombie::SetZombieState(ZombieState* newState)
{
	zombieState = newState;
	stateEnum = zombieState->GetStateEnum();
	MaskToInfoBit(ZIBT::State);
	if (stateEnum == EZombieState::IDLE)
	{
		targetWeakPtr.reset();
		inRangePlayerMap.clear();
	}
}

bool Zombie::CheckNearestPlayer()
{
	if (inRangePlayerMap.empty())
		return false;

	float minDist = 10000.f;
	int nearestNumber = -1;
	for (auto& kv : inRangePlayerMap)
	{
		if (auto player = kv.second.lock())
		{
			Vector3D toTarget = player->GetLocation() - GetLocation();
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
		else
		{
			inRangePlayerMap.erase(kv.first);
		}
	}
	if (nearestNumber == -1)
	{
		return false;
	}
	else
	{
		if (auto shPtr = inRangePlayerMap[nearestNumber].lock())
		{
			if (shPtr->GetIsDead() == false)
			{
				SetTargetPlayer(shPtr);
				return true;
			}
		}
		inRangePlayerMap.erase(nearestNumber);
		CheckNearestPlayer();
		return false;
	}
}

void Zombie::ProcessMovement()
{
	pathManager->ProcessMovement();
}

bool Zombie::IsTargetValid()
{
	if (auto shPtr = targetWeakPtr.lock())
		return true;
	else
		return false;
}

void Zombie::AddMovement(const Vector3D& direction, const Vector3D& dest)
{
	Vector3D location = GetLocation();
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

	SetLocation(location);
	MaskToInfoBit(ZIBT::Location);
}

PathManager* Zombie::GetPathManager()
{
	return pathManager.get();
}

void Zombie::SetRotation(const Rotator& rotation)
{
	Character::SetRotation(rotation);
	MaskToInfoBit(ZIBT::Rotation);
}

void Zombie::SetNextGrid(const Vector3D& nextLoc)
{
	nextLocation = nextLoc;
	MaskToInfoBit(ZIBT::NextLocation);
}

void Zombie::Activate()
{
	isActive = true;
}

void Zombie::Deactivate()
{
	isActive = false;
	InitializeInfo();
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

void Zombie::SetTargetPlayer(shared_ptr<Player> player)
{
	targetWeakPtr = player;
	if (auto targetSharedPtr = targetWeakPtr.lock())
	{
		targetSharedPtr->SetZombieNumberWrestleWith(GetNumber());
		MaskToInfoBit(ZIBT::TargetNumber);
	}
}

std::shared_ptr<Player> Zombie::GetTargetPlayer() const
{
	return targetWeakPtr.lock();
}

bool Zombie::GetTargetLocation(Vector3D& location)
{
	if (auto targetSharedPtr = targetWeakPtr.lock())
	{
		if (targetSharedPtr->GetIsDead() == false)
		{
			location = targetSharedPtr->GetLocation();
			return true;
		}
	}
	return false;
}

void Zombie::CheckTargetAndCancelTargetting(const int playerNumber)
{
	if (auto targetSharedPtr = targetWeakPtr.lock())
	{
		if (targetSharedPtr->GetNumber() == playerNumber)
		{
			SetZombieState(IdleState::GetInstance());
		}
	}
}

void Zombie::SerializeData(ostream& stream)
{
	if (sendInfoBitMask == 0)
		return;

	stream << GetNumber() << "\n";
	stream << sendInfoBitMask << "\n";
	const int bitMax = static_cast<int>(ZIBT::MAX);
	for (int bit = 0; bit < bitMax; bit++)
	{
		if (sendInfoBitMask & (1 << bit))
		{
			SaveInfoToPacket(stream, bit);
			sendInfoBitMask &= ~(1 << bit);
		}
	}
}

void Zombie::TakeDamage(const float damage)
{
	health = max(health - damage, 0.f);
	if (health == 0.f)
	{
		// critical section
		Deactivate();
		zombieDeadCb(GetNumber());
	}
}

void Zombie::SaveInfoToPacket(ostream& stream, const int bitType)
{
	ZIBT type = static_cast<ZIBT>(bitType);
	switch (type)
	{
		case ZIBT::Location:
		{
			SerializeLocation(stream);
			break;
		}
		case ZIBT::Rotation:
		{
			SerializeRotation(stream);
			break;
		}
		case ZIBT::State:
		{
			stream << static_cast<int>(stateEnum) << "\n";
			break;
		}
		case ZIBT::TargetNumber:
		{
			if(auto targetSharedPtr = targetWeakPtr.lock())
				stream << targetSharedPtr->GetNumber() << "\n";
			break;
		}
		case ZIBT::NextLocation:
		{
			stream << nextLocation;
			break;
		}
	}
}

void Zombie::MaskToInfoBit(const ZIBT bitType)
{
	sendInfoBitMask |= (1 << static_cast<int>(bitType));
}

void Zombie::InitializeInfo()
{
	SetZombieState(IdleState::GetInstance());
	targetWeakPtr.reset();
	inRangePlayerMap.clear();
	health = maxHealth;
	sendInfoBitMask = 0;
}

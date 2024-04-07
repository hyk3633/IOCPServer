#include "Player.h"
#include <iostream>
using namespace std;

Player::Player(const int num) : Character(num)
{
}

Player::~Player()
{
}

void Player::InitializePlayerInfo()
{
	SetLocation(Vector3D{ 0, 0, 97.9f });
	SetRotation(Rotator{ 0, 0, 0 });
	velocity = Vector3D{ 0, 0, 0 };
	isSuccessToBlocking = false;
	wrestleState = EWrestleState::ABLE;
	wrestleWaitElapsedTime = 0.f;
	zombieNumberWrestleWith = -1;
	health = maxHealth;
	isDead = false;
}

void Player::SetPlayerID(const string& id)
{
	playerID = id;
}

void Player::WrestleStateOn()
{
	wrestleState = EWrestleState::WRESTLING;
	wrestlingCb(GetNumber());
}

void Player::WrestlStateOff()
{
	wrestleState = EWrestleState::WAITING;
}

void Player::RegisterWrestlingCallback(WrestlingCallback wc)
{
	wrestlingCb = wc;
}

void Player::RegisterPlayerDeadCallback(PlayerDeadCallback pdc)
{
	playerDeadCb = pdc;
}

void Player::SerializeData(ostream& stream)
{
	stream << GetNumber() << "\n";
	SerializeLocation(stream);
	SerializeRotation(stream);
	stream << velocity;
	stream << currentRatencyStart << "\n";
}

void Player::DeserializeData(istream& stream)
{
	DeserializeLocation(stream);
	DeserializeRotation(stream);
	stream >> velocity;
	stream >> currentRatencyStart;
}

void Player::Waiting()
{
	if (wrestleState == EWrestleState::WAITING)
	{
		wrestleWaitElapsedTime += 0.2f;
		if (wrestleWaitElapsedTime >= wrestleWaitTime)
		{
			wrestleWaitElapsedTime = 0.f;
			wrestleState = EWrestleState::ABLE;
		}
	}
}

void Player::TakeDamage(const float damage)
{
	health = max(health - damage, 0.f);
	if (health == 0.f)
	{
		// critical section
		isDead = true;
		playerDeadCb(GetNumber());
	}
}

void Player::SetZombieNumberWrestleWith(const int number)
{
	zombieNumberWrestleWith = number;
}

#include "Player.h"
#include <iostream>
using namespace std;

Player::Player(const int num) : Character(num)
{
}

Player::~Player()
{
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
}

void Player::SerializeExtraData(ostream& stream)
{
	if (sendInfoBitMask == 0)
	{
		stream << false << "\n";
	}
	else
	{
		stream << true << "\n";
		stream << sendInfoBitMask << "\n";
		const int bitMax = static_cast<int>(PIBTC::MAX);
		for (int bit = 0; bit < bitMax; bit++)
		{
			if (sendInfoBitMask & (1 << bit))
			{
				SaveInfoToPacket(stream, bit);
				sendInfoBitMask &= ~(1 << bit);
			}
		}
	}
}

void Player::SaveInfoToPacket(ostream& stream, const int bitType)
{
	PIBTS type = static_cast<PIBTS>(bitType);
	switch (type)
	{
		case PIBTS::WrestlingState:
		{
			break;
		}
	}
}

void Player::DeserializeData(istream& stream)
{
	DeserializeLocation(stream);
	DeserializeRotation(stream);
	stream >> velocity;
}

void Player::DeserializeExtraData(istream& stream)
{
	bool flag = false;
	stream >> flag;
	if (flag == false)
		return;

	stream >> recvInfoBitMask;
	const int bitMax = static_cast<int>(PIBTC::MAX);
	for (int bit = 0; bit < bitMax; bit++)
	{
		if (recvInfoBitMask & (1 << bit))
		{
			ReceiveInfoToPacket(stream, bit);
		}
	}
}

void Player::ReceiveInfoToPacket(istream& stream, const int bitType)
{
	PIBTC type = static_cast<PIBTC>(bitType);
	switch (type)
	{
		case PIBTC::ZombiesInRange:
		{
			zombiesInRange.clear();
			int vectorSize = 0, number = -1;
			stream >> vectorSize;
			for (int i = 0; i < vectorSize; i++)
			{
				stream >> number;
				zombiesInRange.push_back(number);
			}
			break;
		}
		case PIBTC::ZombiesOutRange:
		{
			zombiesOutRange.clear();
			int vectorSize = 0, number = -1;
			stream >> vectorSize;
			for (int i = 0; i < vectorSize; i++)
			{
				stream >> number;
				zombiesOutRange.push_back(number);
			}
			break;
		}
		case PIBTC::ZombieAttackResult:
		{
			stream >> isHit;
			stream >> zombieNumberAttackedBy;
			break;
		}
	}
}

void Player::Waiting()
{
	if (wrestleState == EWrestleState::WAITING)
	{
		wrestleWaitElapsedTime += 0.016f;
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
		playerDeadCb(GetNumber());
	}
}

void Player::SetZombieNumberWrestleWith(const int number)
{
	zombieNumberWrestleWith = number;
}

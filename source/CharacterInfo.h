#pragma once

#include <sstream>
#include <unordered_map>
#include <cmath>
#include "Define.h"
#include "Zombie/State/ZombieState.h"
#include <iostream>
using namespace std;

struct ZombieInfo
{
	Vector3D location;
	Rotator rotation;
	EZombieState state = EZombieState::IDLE;
	int targetNumber;
	Vector3D nextLocation;

	friend istream& operator>>(istream& stream, ZombieInfo& info)
	{
		stream >> info.location;
		return stream;
	}

	friend ostream& operator<<(ostream& stream, ZombieInfo& info)
	{
		stream << info.location;
		stream << info.rotation;
		stream << static_cast<int>(info.state) << "\n";
		stream << info.targetNumber << "\n";
		stream << info.nextLocation;
		return stream;
	}
};

class ZombieInfoSet
{
public:

	ZombieInfoSet() {};
	~ZombieInfoSet() {};

	unordered_map<int, ZombieInfo> zombieInfoMap;

	friend istream& operator>>(istream& stream, ZombieInfoSet& info)
	{
		int characterCount = 0;
		int characterNumber = 0;
		info.zombieInfoMap.clear();

		stream >> characterCount;
		for (int i = 0; i < characterCount; i++)
		{
			stream >> characterNumber;
			stream >> info.zombieInfoMap[characterNumber];
		}
		return stream;
	}

	friend ostream& operator<<(ostream& stream, ZombieInfoSet& info)
	{
		stream << info.zombieInfoMap.size() << "\n";
		for (auto& p : info.zombieInfoMap)
		{
			stream << p.first << "\n";
			stream << p.second << "\n";
		}
		return stream;
	}
};

struct CharacterInfo
{
	Vector3D location;
	Vector3D velocity;
	Rotator rotation;;

	CharacterInfo& operator=(const CharacterInfo& info)
	{
		location = info.location;
		velocity = info.velocity;
		rotation = info.rotation;
		return *this;
	}

	friend istream& operator>>(istream& stream, CharacterInfo& info)
	{
		stream >> info.location;
		stream >> info.velocity;
		stream >> info.rotation;
		return stream;
	}

	friend ostream& operator<<(ostream& stream, CharacterInfo& info)
	{
		stream << info.location;
		stream << info.velocity;
		stream << info.rotation;
		return stream;
	}
};

enum class EWrestleState
{
	ABLE,
	WRESTLING,
	WAITING
};

enum class EPlayerInfoBitTypeClient
{
	UncoveredByZombie,
	ZombieAttackResult,
	WrestlingResult,
	WrestlingEnd,
	MAX
};

typedef EPlayerInfoBitTypeClient PIBTC;

enum class EPlayerInfoBitTypeServer
{
	WrestlingState,
	PlayGrabReaction,
	MAX
};

typedef EPlayerInfoBitTypeServer PIBTS;

struct PlayerInfo
{
	// 필수 데이터
	CharacterInfo characterInfo;
	int recvInfoBitMask;

	// 클라이언트 수신용 데이터
	vector<int> zombiesWhoSawMe;
	bool isHitted;
	int zombieNumberAttackedMe;
	bool isSuccessToBlocking;

	// 클라이언트 전송용 데이터
	int sendInfoBitMask;
	EWrestleState wrestleState = EWrestleState::ABLE;
	bool isBlockingAction;

	float wrestleWaitTime = 5.f;
	float wrestleWaitElapsedTime = 0.f;

	friend ostream& operator<<(ostream& stream, PlayerInfo& info)
	{
		stream << info.characterInfo;
		stream << info.sendInfoBitMask << "\n";
		const int bitMax = static_cast<int>(PIBTC::MAX);
		for (int bit = 0; bit < bitMax; bit++)
		{
			if (info.sendInfoBitMask & (1 << bit))
			{
				SaveInfoToPacket(stream, info, bit);
				info.sendInfoBitMask &= ~(1 << bit);
			}
		}
		return stream;
	}

	friend void SaveInfoToPacket(std::ostream& stream, PlayerInfo& info, const int bitType)
	{
		PIBTS type = static_cast<PIBTS>(bitType);
		switch (type)
		{
			case PIBTS::WrestlingState:
			{
				stream << static_cast<int>(info.wrestleState) << "\n";
				break;
			}
			case PIBTS::PlayGrabReaction:
			{
				stream << info.isBlockingAction << "\n";
				break;
			}
		}
	}

	friend istream& operator>>(istream& stream, PlayerInfo& info)
	{
		stream >> info.characterInfo;
		stream >> info.recvInfoBitMask;
		const int bitMax = static_cast<int>(PIBTC::MAX);
		for (int bit = 0; bit < bitMax; bit++)
		{
			if (info.recvInfoBitMask & (1 << bit))
			{
				info.ReceiveInfoToPacket(stream, bit);
			}
		}
		return stream;
	}

	void ReceiveInfoToPacket(istream& stream, const int bitType)
	{
		PIBTC type = static_cast<PIBTC>(bitType);
		switch (type)
		{
			case PIBTC::UncoveredByZombie:
			{
				zombiesWhoSawMe.clear();
				int vectorSize = 0, number = -1;
				stream >> vectorSize;
				for (int i = 0; i < vectorSize; i++)
				{
					stream >> number;
					zombiesWhoSawMe.push_back(number);
				}
				break;
			}
			case PIBTC::ZombieAttackResult:
			{
				stream >> isHitted;
				stream >> zombieNumberAttackedMe;
				break;
			}
			case PIBTC::WrestlingResult:
			{
				stream >> isSuccessToBlocking;
				break;
			}
			case PIBTC::WrestlingEnd:
			{
				break;
			}
		}
	}
};

class PlayerInfoSet
{
public:

	PlayerInfoSet() {};
	~PlayerInfoSet() {};

	unordered_map<int, PlayerInfo> characterInfoMap;

	friend istream& operator>>(istream& stream, PlayerInfoSet& info)
	{
		int characterCount = 0;
		int characterNumber = 0;
		info.characterInfoMap.clear();

		stream >> characterCount;
		for (int i = 0; i < characterCount; i++)
		{
			stream >> characterNumber;
			stream >> info.characterInfoMap[characterNumber];
		}
		return stream;
	}

	friend ostream& operator<<(ostream& stream, PlayerInfoSet& info)
	{
		stream << info.characterInfoMap.size() << "\n";
		for (auto& p : info.characterInfoMap)
		{
			stream << p.first << "\n";
			stream << p.second << "\n";
		}
		return stream;
	}
};

class PlayerInfoSetEx : public PlayerInfoSet
{
public:

	PlayerInfoSetEx() {};
	~PlayerInfoSetEx() {};

	unordered_map<int, string> playerIDMap;

	void InputStreamWithID(istream& stream)
	{
		int playerCount = 0;
		int playerNumber = 0;

		stream >> playerCount;
		for (int i = 0; i < playerCount; i++)
		{
			stream >> playerNumber;
			stream >> playerIDMap[playerNumber];
			stream >> characterInfoMap[playerNumber];
		}
	}

	void OutputStreamWithID(ostream& stream)
	{
		stream << characterInfoMap.size() << "\n";	// 플레이어 수
		for (auto& p : characterInfoMap)
		{
			stream << p.first << "\n";					// 플레이어 번호
			stream << playerIDMap[p.first] << "\n";		// 플레이어 아이디
			stream << p.second.characterInfo << "\n";	// 플레이어 정보 구조체
		}
	}
};
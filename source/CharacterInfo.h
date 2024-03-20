#pragma once

#include <sstream>
#include <unordered_map>
#include <cmath>
#include "Define.h"
#include "Zombie/State/ZombieState.h"
#include "Structs/Vector3D.h"
#include "Structs/Rotator.h"
#include <iostream>

using namespace std;

enum class EZombieInfoBitType
{
	Location,
	Rotation,
	State,
	TargetNumber,
	NextLocation,
	MAX
};

typedef EZombieInfoBitType ZIBT;

struct ZombieInfo
{
	int sendInfoBitMask;

	Vector3D location;
	Rotator rotation;
	EZombieState state = EZombieState::IDLE;
	int targetNumber;
	Vector3D nextLocation;

	friend ostream& operator<<(ostream& stream, ZombieInfo& info)
	{
		stream << info.sendInfoBitMask << "\n";

		const int bitMax = static_cast<int>(ZIBT::MAX);
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

	friend void SaveInfoToPacket(std::ostream& stream, ZombieInfo& info, const int bitType)
	{
		ZIBT type = static_cast<ZIBT>(bitType);
		switch (type)
		{
			case ZIBT::Location:
			{
				stream << info.location;
				break;
			}
			case ZIBT::Rotation:
			{
				stream << info.rotation;
				break;
			}
			case ZIBT::State:
			{
				stream << static_cast<int>(info.state) << "\n";
				break;
			}
			case ZIBT::TargetNumber:
			{
				stream << info.targetNumber << "\n";
				break;
			}
			case ZIBT::NextLocation:
			{
				stream << info.nextLocation;
				break;
			}
		}
	}
};

class ZombieInfoSet
{
public:

	ZombieInfoSet() {};
	~ZombieInfoSet() {};

	unordered_map<int, ZombieInfo> zombieInfoMap;

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
	Rotator rotation;

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



enum class EPlayerInfoBitTypeClient
{
	ZombiesInRange,
	ZombiesOutRange,
	ZombieAttackResult,
	MAX
};

typedef EPlayerInfoBitTypeClient PIBTC;

enum class EPlayerInfoBitTypeServer
{
	WrestlingState,
	MAX
};

typedef EPlayerInfoBitTypeServer PIBTS;

struct PlayerInfo
{
	// 필수 데이터
	CharacterInfo characterInfo;
	int recvInfoBitMask;

	// 클라이언트 수신용 데이터
	vector<int> zombiesInRange, zombiesOutRange;
	bool isHitted;
	int zombieNumberAttackedMe;

	// 클라이언트 전송용 데이터
	int sendInfoBitMask;

	bool isSuccessToBlocking;
	EWrestleState wrestleState = EWrestleState::ABLE;
	float wrestleWaitTime = 5.f;
	float wrestleWaitElapsedTime = 0.f;

	friend ostream& operator<<(ostream& stream, PlayerInfo& info)
	{
		stream << info.characterInfo;
		stream << info.sendInfoBitMask << "\n";
		//const int bitMax = static_cast<int>(PIBTC::MAX);
		//for (int bit = 0; bit < bitMax; bit++)
		//{
		//	if (info.sendInfoBitMask & (1 << bit))
		//	{
		//		SaveInfoToPacket(stream, info, bit);
		//		info.sendInfoBitMask &= ~(1 << bit);
		//	}
		//}
		return stream;
	}

	friend void SaveInfoToPacket(std::ostream& stream, PlayerInfo& info, const int bitType)
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
				stream >> isHitted;
				stream >> zombieNumberAttackedMe;
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
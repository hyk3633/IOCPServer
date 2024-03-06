#pragma once

#include <sstream>
#include <unordered_map>
#include <cmath>
#include "Define.h"
#include "Zombie/State/ZombieState.h"

using namespace std;

struct CharacterInfo
{
	Vector3D location;
	float velocityX, velocityY, velocityZ;
	float pitch, yaw, roll;
	
	CharacterInfo& operator=(const CharacterInfo& info)
	{
		location = info.location;
		velocityX = info.velocityX;
		velocityY = info.velocityY;
		velocityZ = info.velocityZ;
		pitch = info.pitch;
		yaw = info.yaw;
		roll = info.roll;
		return *this;
	}

	friend istream& operator>>(istream& stream, CharacterInfo& info)
	{
		stream >> info.location.X >> info.location.Y >> info.location.Z;
		stream >> info.velocityX >> info.velocityY >> info.velocityZ;
		stream >> info.pitch >> info.yaw >> info.roll;
		return stream;
	}

	friend ostream& operator<<(ostream& stream, CharacterInfo& info)
	{
		stream << info.location.X << "\n" << info.location.Y << "\n" << info.location.Z << "\n";
		stream << info.velocityX << "\n" << info.velocityY << "\n" << info.velocityZ << "\n";
		stream << info.pitch << "\n" << info.yaw << "\n" << info.roll << "\n";
		return stream;
	}
};

struct PlayerInfo
{
	CharacterInfo characterInfo;
	bool isZombiesSawMe;
	vector<int> zombiesWhoSawMe;

	friend istream& operator>>(istream& stream, PlayerInfo& info)
	{
		stream >> info.characterInfo;
		stream >> info.isZombiesSawMe;
		info.zombiesWhoSawMe.clear();
		if (info.isZombiesSawMe)
		{
			int vectorSize = 0, number = -1;
			stream >> vectorSize;
			for (int i = 0; i < vectorSize; i++)
			{
				stream >> number;
				info.zombiesWhoSawMe.push_back(number);
			}
		}
		return stream;
	}
};

struct ZombieInfo
{
	CharacterInfo characterInfo;
	EZombieState state;
	bool bSetPath;
	vector<Pos> pathToTarget;
	int targetNumber;

	friend istream& operator>>(istream& stream, ZombieInfo& info)
	{
		stream >> info.characterInfo;
		return stream;
	}

	friend ostream& operator<<(ostream& stream, ZombieInfo& info)
	{
		stream << info.characterInfo;
		stream << static_cast<int>(info.state) << "\n";
		stream << info.targetNumber << "\n";
		stream << info.bSetPath << "\n";
		if (info.bSetPath)
		{
			stream << info.pathToTarget.size() << "\n";
			for (Pos& pos : info.pathToTarget)
			{
				stream << pos.x << "\n" << pos.y << "\n";
			}
			info.bSetPath = false;
		}
		
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
			stream << p.second.characterInfo << "\n";
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
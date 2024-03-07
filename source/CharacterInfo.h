#pragma once

#include <sstream>
#include <unordered_map>
#include <cmath>
#include "Define.h"
#include "Zombie/State/ZombieState.h"

using namespace std;

struct ZombieInfo
{
	Vector3D location;
	Rotator rotation;
	EZombieState state = EZombieState::IDLE;
	int targetNumber;
	bool bSetPath;
	vector<Pos> pathToTarget;

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
		stream << characterInfoMap.size() << "\n";	// �÷��̾� ��
		for (auto& p : characterInfoMap)
		{
			stream << p.first << "\n";					// �÷��̾� ��ȣ
			stream << playerIDMap[p.first] << "\n";		// �÷��̾� ���̵�
			stream << p.second.characterInfo << "\n";	// �÷��̾� ���� ����ü
		}
	}
};
#pragma once

#include <sstream>
#include <unordered_map>

using namespace std;

struct CharacterInfo
{
	float vectorX, vectorY, vectorZ;
	float velocityX, velocityY, velocityZ;
	float pitch, yaw, roll;

	CharacterInfo& operator=(const CharacterInfo& info)
	{
		vectorX = info.vectorX;
		vectorY = info.vectorY;
		vectorZ = info.vectorZ;
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
		stream >> info.vectorX >> info.vectorY >> info.vectorZ;
		stream >> info.velocityX >> info.velocityY >> info.velocityZ;
		stream >> info.pitch >> info.yaw >> info.roll;
		return stream;
	}

	friend ostream& operator<<(ostream& stream, CharacterInfo& info)
	{
		stream << info.vectorX << "\n" << info.vectorY << "\n" << info.vectorZ << "\n";
		stream << info.velocityX << "\n" << info.velocityY << "\n" << info.velocityZ << "\n";
		stream << info.pitch << "\n" << info.yaw << "\n" << info.roll << "\n";
		return stream;
	}
};

class CharacterInfoSet
{
public:

	CharacterInfoSet() {};
	~CharacterInfoSet() {};

	unordered_map<int, CharacterInfo> characterInfoMap;

	friend istream& operator>>(istream& stream, CharacterInfoSet& info)
	{
		int characterCount = 0;
		int characterNumber = 0;
		CharacterInfo characterInfo{};
		info.characterInfoMap.clear();

		stream >> characterCount;
		for (int i = 0; i < characterCount; i++)
		{
			stream >> characterNumber;
			stream >> characterInfo;
			info.characterInfoMap[characterNumber] = characterInfo;
		}
		return stream;
	}

	friend ostream& operator<<(ostream& stream, CharacterInfoSet& info)
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

class PlayerInfoSetEx : public CharacterInfoSet
{
public:

	PlayerInfoSetEx() {};
	~PlayerInfoSetEx() {};

	unordered_map<int, string> playerIDMap;

	void InputStreamWithID(istream& stream)
	{
		int playerCount = 0;
		string playerID = "";
		int playerNumber = 0;
		CharacterInfo playerInfo{};

		stream >> playerCount;
		for (int i = 0; i < playerCount; i++)
		{
			stream >> playerNumber;
			stream >> playerID;
			stream >> playerInfo;
			playerIDMap[playerNumber] = playerID;
			characterInfoMap[playerNumber] = playerInfo;
		}
	}

	void OutputStreamWithID(ostream& stream)
	{
		stream << characterInfoMap.size() << "\n";		// 플레이어 수
		for (auto& p : characterInfoMap)
		{
			stream << p.first << "\n";				// 플레이어 번호
			stream << playerIDMap[p.first] << "\n";	// 플레이어 아이디
			stream << p.second << "\n";				// 플레이어 정보 구조체
		}
	}
};


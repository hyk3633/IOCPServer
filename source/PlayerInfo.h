#pragma once

#include <sstream>
#include <unordered_map>

using namespace std;

struct PlayerInfo
{
	float vectorX, vectorY, vectorZ;
	float velocityX, velocityY, velocityZ;
	float pitch, yaw, roll;

	PlayerInfo& operator=(const PlayerInfo& info)
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

	friend istream& operator>>(istream& stream, PlayerInfo& info)
	{
		stream >> info.vectorX >> info.vectorY >> info.vectorZ;
		stream >> info.velocityX >> info.velocityY >> info.velocityZ;
		stream >> info.pitch >> info.yaw >> info.roll;
		return stream;
	}

	friend ostream& operator<<(ostream& stream, PlayerInfo& info)
	{
		stream << info.vectorX << "\n" << info.vectorY << "\n" << info.vectorZ << "\n";
		stream << info.velocityX << "\n" << info.velocityY << "\n" << info.velocityZ << "\n";
		stream << info.pitch << "\n" << info.yaw << "\n" << info.roll << "\n";
		return stream;
	}
};

class PlayerInfoSet
{
public:

	PlayerInfoSet() {};
	~PlayerInfoSet() {};

	unordered_map<int, PlayerInfo> playerInfoMap;

	friend istream& operator>>(istream& stream, PlayerInfoSet& info)
	{
		int playerCount = 0;
		int playerNumber = 0;
		PlayerInfo playerInfo{};
		info.playerInfoMap.clear();

		stream >> playerCount;
		for (int i = 0; i < playerCount; i++)
		{
			stream >> playerNumber;
			stream >> playerInfo;
			info.playerInfoMap[playerNumber] = playerInfo;
		}
		return stream;
	}

	friend ostream& operator<<(ostream& stream, PlayerInfoSet& info)
	{
		stream << info.playerInfoMap.size() << "\n";
		for (auto& p : info.playerInfoMap)
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
		string playerID = "";
		int playerNumber = 0;
		PlayerInfo playerInfo{};

		stream >> playerCount;
		for (int i = 0; i < playerCount; i++)
		{
			stream >> playerNumber;
			stream >> playerID;
			stream >> playerInfo;
			playerIDMap[playerNumber] = playerID;
			playerInfoMap[playerNumber] = playerInfo;
		}
	}

	void OutputStreamWithID(ostream& stream)
	{
		stream << playerInfoMap.size() << "\n";		// 플레이어 수
		for (auto& p : playerInfoMap)
		{
			stream << p.first << "\n";				// 플레이어 번호
			stream << playerIDMap[p.first] << "\n";	// 플레이어 아이디
			stream << p.second << "\n";				// 플레이어 정보 구조체
		}
	}
};


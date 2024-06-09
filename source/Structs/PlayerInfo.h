#pragma once

struct PlayerInfo
{
	int health;
	int row;
	int column;
	float kickPower;

	PlayerInfo() {}
	PlayerInfo(int h, int r, int c, float k) : health(h), row(r), column(c), kickPower(k) {}
	PlayerInfo& operator=(const PlayerInfo& info)
	{
		*this = info;
		return *this;
	}
};
#pragma once

struct PlayerInfo
{
	int health;
	int row;
	int column;

	PlayerInfo() {}
	PlayerInfo(int h, int r, int c) : health(h), row(r), column(c) {}
	PlayerInfo& operator=(const PlayerInfo& info)
	{
		*this = info;
		return *this;
	}
};
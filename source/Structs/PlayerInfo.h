#pragma once

struct PlayerInfo
{
	int health;
	int stamina;
	int row;
	int column;

	PlayerInfo() {}
	PlayerInfo(int h, int r, int c, int s) : health(h), row(r), column(c), stamina(s) {}
	PlayerInfo& operator=(const PlayerInfo& info)
	{
		*this = info;
		return *this;
	}
};
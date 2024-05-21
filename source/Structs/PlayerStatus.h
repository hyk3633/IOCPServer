#pragma once
#include <fstream>

using std::ostream;

struct PlayerStatus
{
	float health;

	friend ostream& operator<<(ostream& stream, PlayerStatus& status)
	{
		stream << status.health << "\n";
		return stream;
	}
};
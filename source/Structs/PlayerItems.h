#pragma once
#include <fstream>

using std::ostream;

struct PlayerItems
{
	int itemID;
	int quantity;
	int topLeftX;
	int topLeftY;
	int isRotated;
	int isEquipped;
	int equippedSlotNumber;

	friend ostream& operator<<(ostream& stream, PlayerItems& info)
	{
		stream << info.itemID << "\n";
		stream << info.quantity << "\n";
		stream << info.topLeftX << "\n";
		stream << info.topLeftY << "\n";
		stream << info.isRotated << "\n";
		stream << info.isEquipped << "\n";
		stream << info.equippedSlotNumber << "\n";
		return stream;
	}
};
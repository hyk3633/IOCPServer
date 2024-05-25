#pragma once
#include <string>
#include <fstream>

struct EquippedItem
{
	std::string itemID;
	int itemKey;
	int slotNumber;
	bool isArmed = false;
	friend ostream& operator<<(ostream& stream, EquippedItem& equipped)
	{
		stream << equipped.itemID << "\n";
		stream << equipped.itemKey << "\n";
		stream << equipped.slotNumber << "\n";
		stream << equipped.isArmed << "\n";
		return stream;
	}
};
#pragma once
#include <string>
#include <fstream>

struct EquippedItem
{
	std::string itemID;
	int itemKey;
	int slotNumber;
	friend ostream& operator<<(ostream& stream, EquippedItem& equipped)
	{
		stream << equipped.itemID << "\n";
		stream << equipped.itemKey << "\n";
		stream << equipped.slotNumber << "\n";
		return stream;
	}
};
#pragma once
#include <string>
#include <fstream>
struct PossessedItem
{
	std::string itemID;
	int itemKey;
	int quantity;
	int isRotated;
	int topLeftX;
	int topLeftY;
	friend std::ostream& operator<<(std::ostream& stream, PossessedItem& possessed)
	{
		stream << possessed.itemID << "\n";
		stream << possessed.itemKey << "\n";
		stream << possessed.quantity << "\n";
		stream << possessed.isRotated << "\n";
		stream << possessed.topLeftX << "\n";
		stream << possessed.topLeftY << "\n";
		return stream;
	}
};
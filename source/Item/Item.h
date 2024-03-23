#pragma once
#include "../Enums/ItemType.h"
#include "../Structs/Vector3D.h"

union ItemSubType
{
	EMeleeWeaponType meleeWeaponType;
};

struct Item
{
	EItemState state;
	EItemMainType mainType;
	ItemSubType itemSubType;
	Vector3D location;

	friend std::ostream& operator<<(std::ostream& stream, Item* item)
	{
		stream << static_cast<int>(item->state) << "\n";
		stream << static_cast<int>(item->mainType) << "\n";
		stream << static_cast<int>(item->itemSubType.meleeWeaponType) << "\n"; // 타입에 따라 다르게 넣도록
		stream << item->location;
		return stream;
	}
};
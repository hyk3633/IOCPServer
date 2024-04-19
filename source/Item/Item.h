#pragma once
#include "../Enums/ItemType.h"
#include "../Structs/ItemInfo.h"
#include "../Structs/Vector3D.h"
#include <memory>

union ItemSubType
{
	EMeleeWeaponType meleeWeaponType;
};

struct Item
{
	EItemState state;
	std::shared_ptr<ItemInfo> itemInfo;
	Vector3D location;

	Item() : state(EItemState::Deactivated), itemInfo(nullptr), location() {}
	Item(EItemState _state, std::shared_ptr<ItemInfo> _info, Vector3D _loc) : state(_state), itemInfo(_info), location(_loc) {}

	friend std::ostream& operator<<(std::ostream& stream, shared_ptr<Item> item)
	{
		//stream << static_cast<int>(item->state) << "\n";
		stream << item->itemInfo->itemKey << "\n";
		stream << item->location;
		return stream;
	}
};
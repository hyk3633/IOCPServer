#pragma once
#include "../Enums/ItemType.h"
#include "../Structs/ItemInfo.h"
#include "../Structs/Vector3D.h"
#include "../Structs/GridPoint.h"
#include <sstream>
#include <memory>

struct Item
{
	EItemState state;
	ItemInfo itemInfo;
	Vector3D location;
	bool isRotated = false;
	GridPoint gridPoint;
	
	std::stringstream concreteInfoStream;

	Item() : state(EItemState::Deactivated), itemInfo(), location() {}
	Item(EItemState _state, ItemInfo& _info, std::stringstream& concInfo, Vector3D _loc)
		: state(_state), itemInfo(_info), location(_loc) { concreteInfoStream = std::move(concInfo); }

	void Rotate()
	{
		isRotated = !isRotated;
		itemInfo.itemGridSize = { itemInfo.itemGridSize.y, itemInfo.itemGridSize.x };
	}

	friend std::ostream& operator<<(std::ostream& stream, shared_ptr<Item> item)
	{ 
		//stream << static_cast<int>(item->state) << "\n";
		stream << item->itemInfo.itemKey << "\n";
		stream << item->location;
		return stream;
	}
};
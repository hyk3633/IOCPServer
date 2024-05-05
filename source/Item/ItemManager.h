#pragma once
#include "Item.h"
#include "../Structs/ItemInfo.h"
#include "../Structs/GridPoint.h"
#include "../Structs/Vector3D.h"
#include <memory>
#include <unordered_map>
#include <sstream>
#include <mutex>

class JsonComponent;

class ItemManager
{
public:

	ItemManager();
	~ItemManager() = default;

	shared_ptr<Item> GetItem(const int itemID);

	void SetItemStateToActivated(const int itemNumber);

	void SetItemStateToDeactivated(const int itemNumber);

	void SaveItemInfoToPacket(std::ostream& stream);

	// 지정된 위치에 아이템 생성하는 함수

private:

	unique_ptr<JsonComponent> jsonComponent;

	std::unordered_map<int, shared_ptr<Item>> itemMap;

	std::unordered_map<int, weak_ptr<Item>> deactivatedItemMap;

	std::unordered_map<int, weak_ptr<Item>> activatedItemMap;

	std::mutex itemMutex;

};
#pragma once
#include "Item.h"
#include "../Structs/ItemInfo.h"
#include "../Structs/GridPoint.h"
#include "../Structs/Vector3D.h"
#include "../Structs/PossessedItem.h"
#include "../Structs/EquippedItem.h"
#include "../Enums/ItemFunctions.h"
#include <memory>
#include <unordered_map>
#include <sstream>
#include <mutex>
#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")

struct Item;
class JsonComponent;
class Player;

typedef void(*ItemDestroyCallback)(const int, shared_ptr<Item>, const string&);

class ItemManager
{
public:

	ItemManager(ItemDestroyCallback idc);
	~ItemManager() = default;

	shared_ptr<Item> GetItem(const string& itemID);
	
	void RemoveItem(const string& itemID);

	void SetItemStateToActivated(const string& itemID);

	void SetItemStateToDeactivated(const string& itemID);

	void SaveItemInfoToPacket(std::ostream& stream);

	void MakePlayersPossessedItems(const vector<PossessedItem>& possessedItems);

	void MakePlayersEquippedItems(const vector<EquippedItem>& equippedItems);

	void UseItem(std::shared_ptr<Player> player, const string& itemID, const int usedAmount);

protected:

	void DestroyItem(const int playerNumber, const string& itemID);

	// 지정된 위치에 아이템 생성하는 함수

protected:

	string MakeItemUUID();

	void Healing(std::shared_ptr<Player> player, const string& itemID);

private:

	unique_ptr<JsonComponent> jsonComponent;

	std::unordered_map<string, shared_ptr<Item>> itemMap;

	std::unordered_map<string, weak_ptr<Item>> deactivatedItemMap;

	std::unordered_map<string, weak_ptr<Item>> activatedItemMap;

	std::unordered_map<EItemMainType, void (ItemManager::*)(std::shared_ptr<Player>, const string&)> itemFunction;

	std::mutex itemMutex;

	UUID uuid;

	char* itemUuid;

	ItemDestroyCallback itemDestroyCb = nullptr;

};
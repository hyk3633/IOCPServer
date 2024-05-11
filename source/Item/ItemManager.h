#pragma once
#include "Item.h"
#include "../Structs/ItemInfo.h"
#include "../Structs/GridPoint.h"
#include "../Structs/Vector3D.h"
#include "../Structs/PossessedItem.h"
#include "../Structs/EquippedItem.h"
#include <memory>
#include <unordered_map>
#include <sstream>
#include <mutex>
#include <Rpc.h>
#pragma comment(lib, "Rpcrt4.lib")

class JsonComponent;

class ItemManager
{
public:

	ItemManager();
	~ItemManager() = default;

	shared_ptr<Item> GetItem(const string& itemID);
	
	void RemoveItem(const string& itemID);

	void SetItemStateToActivated(const string& itemID);

	void SetItemStateToDeactivated(const string& itemID);

	void SaveItemInfoToPacket(std::ostream& stream);

	void MakePlayersPossessedItems(const vector<PossessedItem>& possessedItems);

	void MakePlayersEquippedItems(const vector<EquippedItem>& equippedItems);

	// ������ ��ġ�� ������ �����ϴ� �Լ�

protected:

	string MakeItemUUID();

private:

	unique_ptr<JsonComponent> jsonComponent;

	std::unordered_map<string, shared_ptr<Item>> itemMap;

	std::unordered_map<string, weak_ptr<Item>> deactivatedItemMap;

	std::unordered_map<string, weak_ptr<Item>> activatedItemMap;

	std::mutex itemMutex;

	UUID uuid;

	char* itemUuid;

};
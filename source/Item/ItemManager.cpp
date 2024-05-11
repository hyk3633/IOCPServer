#include "ItemManager.h"
#include "../JsonComponent.h"

using namespace std;

ItemManager::ItemManager()
{
	jsonComponent = make_unique<JsonComponent>();
	jsonComponent->Initialize();

	stringstream concreteInfoStream;
	ItemInfo itemInfo;

	for (int i = 0; i < 4; i++)
	{
		const string itemID = MakeItemUUID();
		jsonComponent->GetItemCommonInfo(i, itemInfo);
		jsonComponent->GetItemConcreteInfo(i, itemInfo.itemType, concreteInfoStream);
		itemMap[itemID] = make_shared<Item>(EItemState::Activated, itemInfo, concreteInfoStream, Vector3D{ 310,-220 + (float)i * 50,40 });

		activatedItemMap[itemID] = itemMap[itemID];

		concreteInfoStream.str() = "";
	}
}

shared_ptr<Item> ItemManager::GetItem(const string& itemID)
{
	return itemMap[itemID];
}

void ItemManager::RemoveItem(const string& itemID)
{
	itemMap.erase(itemID);
}

void ItemManager::SetItemStateToActivated(const string& itemID)
{
	lock_guard<mutex> lock(itemMutex);
	itemMap[itemID]->state = EItemState::Activated;
	if (deactivatedItemMap.find(itemID) != deactivatedItemMap.end())
	{
		deactivatedItemMap.erase(itemID);
	}
	activatedItemMap[itemID] = itemMap[itemID];
}

void ItemManager::SetItemStateToDeactivated(const string& itemID)
{
	lock_guard<mutex> lock(itemMutex);
	itemMap[itemID]->state = EItemState::Deactivated;
	if (activatedItemMap.find(itemID) != activatedItemMap.end())
	{
		activatedItemMap.erase(itemID);
	}
	deactivatedItemMap[itemID] = itemMap[itemID];
}

void ItemManager::SaveItemInfoToPacket(std::ostream& stream)
{
	lock_guard<mutex> lock(itemMutex);

	stream << itemMap.size() << "\n";
	for (auto& kv : itemMap)
	{
		stream << kv.first << "\n";
		stream << kv.second;
	}
}

void ItemManager::MakePlayersPossessedItems(const vector<PossessedItem>& possessedItems)
{
	stringstream concreteInfoStream;
	ItemInfo itemInfo;

	for (auto& possessed : possessedItems)
	{
		jsonComponent->GetItemCommonInfo(possessed.itemKey, itemInfo);
		jsonComponent->GetItemConcreteInfo(possessed.itemKey, itemInfo.itemType, concreteInfoStream);

		itemInfo.count = possessed.quantity;

		itemMap[possessed.itemID] = make_shared<Item>(EItemState::Acquired, itemInfo, concreteInfoStream, Vector3D{ 0,0,0 });
		
		if (possessed.isRotated)
			itemMap[possessed.itemID]->Rotate();

		concreteInfoStream.str() = "";
	}

	// deactivate ?
	//activatedItemMap[itemID] = itemMap[itemID];
}

void ItemManager::MakePlayersEquippedItems(const vector<EquippedItem>& equippedItems)
{
	stringstream concreteInfoStream;
	ItemInfo itemInfo;

	for (auto& equipped : equippedItems)
	{
		jsonComponent->GetItemCommonInfo(equipped.itemKey, itemInfo);
		jsonComponent->GetItemConcreteInfo(equipped.itemKey, itemInfo.itemType, concreteInfoStream);

		itemMap[equipped.itemID] = make_shared<Item>(EItemState::Acquired, itemInfo, concreteInfoStream, Vector3D{ 0,0,0 });

		concreteInfoStream.str() = "";
	}

	// deactivate ?
	//activatedItemMap[itemID] = itemMap[itemID];
}

string ItemManager::MakeItemUUID()
{
	UuidCreate(&uuid);
	UuidToStringA(&uuid, (RPC_CSTR*)&itemUuid);
	return itemUuid;
}


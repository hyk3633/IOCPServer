#include "ItemManager.h"
#include "../Player/Player.h"
#include "../JsonComponent.h"

using namespace std;

ItemManager::ItemManager(ItemDestroyCallback idc)
{
	itemDestroyCb = idc;

	jsonComponent = make_unique<JsonComponent>();
	jsonComponent->Initialize();

	stringstream concreteInfoStream;
	ItemInfo itemInfo;

	vector<pair<Vector3D, int>> itemPlacedInfos;

	jsonComponent->GetPlacedItemInfo(itemPlacedInfos);
	for (int i = 0; i < itemPlacedInfos.size(); i++)
	{
		const string itemID = MakeItemUUID();
		jsonComponent->GetItemCommonInfo(itemPlacedInfos[i].second, itemInfo);
		jsonComponent->GetItemConcreteInfo(itemPlacedInfos[i].second, itemInfo.itemType, concreteInfoStream);
		itemMap[itemID] = make_shared<Item>(EItemState::Activated, itemInfo, concreteInfoStream, itemPlacedInfos[i].first);

		activatedItemMap[itemID] = itemMap[itemID];

		concreteInfoStream.str() = "";
	}

	itemFunction[EItemMainType::RecoveryItem] = &ItemManager::Healing;
}

void ItemManager::GetPlayerInfo(PlayerInfo& info)
{
	jsonComponent->GetPlayerInfo(info);
}

shared_ptr<Item> ItemManager::GetItem(const string& itemID)
{
	if (itemMap.find(itemID) != itemMap.end())
		return itemMap[itemID];
	else 
		return nullptr;
}

float ItemManager::GetWeaponAttackPower(const string& itemID)
{
	auto item = GetItem(itemID);
	const EItemMainType itemType = item->itemInfo.itemType;
	if (itemType == EItemMainType::MeleeWeapon)
	{
		return WeaponInfo::GetConcreteInfo(item->concreteInfoStream).attackPower;
	}
	else if (itemType == EItemMainType::RangedWeapon)
	{
		return RangedWeaponInfo::GetConcreteInfo(item->concreteInfoStream).attackPower;
	}
	return 0.f;
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

	stream << activatedItemMap.size() << "\n";
	for (auto& kv : activatedItemMap)
	{
		if (auto shPtr = kv.second.lock())
		{
			stream << kv.first << "\n";
			stream << shPtr;
		}
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

		itemInfo.quantity = possessed.quantity;

		itemMap[possessed.itemID] = make_shared<Item>(EItemState::Acquired, itemInfo, concreteInfoStream, Vector3D{ 0,0,0 });
		
		if (possessed.isRotated)
			itemMap[possessed.itemID]->Rotate();

		concreteInfoStream.str() = "";
	}
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
}

void ItemManager::UseItem(shared_ptr<Player> player, const string& itemID, const int consumedAmount)
{
	if (itemMap.find(itemID) != itemMap.end())
	{
		shared_ptr<Item> item = itemMap[itemID];
		if (itemFunction.find(item->itemInfo.itemType) != itemFunction.end())
		{
			(this->*itemFunction[item->itemInfo.itemType])(player, itemID);
		}

		int itemQuantity = item->itemInfo.quantity;
		item->itemInfo.quantity = max(itemQuantity - consumedAmount, 0);
		if (item->itemInfo.quantity == 0)
		{
			DestroyItem(player->GetNumber(), itemID);
		}
	}
	else
	{
		cout << "[ERROR] : �������� �ʴ� ������ ID\n";
	}
}

void ItemManager::DestroyItem(const int playerNumber, const string& itemID)
{
	itemDestroyCb(playerNumber, itemMap[itemID], itemID);
	if (deactivatedItemMap.find(itemID) != deactivatedItemMap.end())
	{
		deactivatedItemMap.erase(itemID);
	}
	itemMap.erase(itemID);
	cout << "[Log] ������ ���� : " << itemID << endl;
}

string ItemManager::MakeItemUUID()
{
	UuidCreate(&uuid);
	UuidToStringA(&uuid, (RPC_CSTR*)&itemUuid);
	return itemUuid;
}

void ItemManager::Healing(std::shared_ptr<Player> player, const string& itemID)
{
	RecoveryItemInfo recoveryItemInfo;
	ParseInfo(itemMap[itemID]->concreteInfoStream, recoveryItemInfo);
	player->Heal(recoveryItemInfo.recoveryAmount);
	cout << "[Log] : �÷��̾� " << player->GetPlayerID() << " �� ü�� " << recoveryItemInfo.recoveryAmount << " ȸ��\n";
}


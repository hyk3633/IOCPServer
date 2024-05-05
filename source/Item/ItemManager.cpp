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
		jsonComponent->GetItemCommonInfo(i, itemInfo);
		jsonComponent->GetItemConcreteInfo(i, itemInfo.itemType, concreteInfoStream);
		itemMap[i] = make_shared<Item>(EItemState::Activated, itemInfo, concreteInfoStream, Vector3D{ 310,-220 + (float)i * 50,40 });

		activatedItemMap[i] = itemMap[i];

		concreteInfoStream.str() = "";
	}
}

shared_ptr<Item> ItemManager::GetItem(const int itemID)
{
	return itemMap[itemID];
}

void ItemManager::SetItemStateToActivated(const int itemNumber)
{
	lock_guard<mutex> lock(itemMutex);
	itemMap[itemNumber]->state = EItemState::Activated;
	if (deactivatedItemMap.find(itemNumber) != deactivatedItemMap.end())
	{
		deactivatedItemMap.erase(itemNumber);
	}
	activatedItemMap[itemNumber] = itemMap[itemNumber];
}

void ItemManager::SetItemStateToDeactivated(const int itemNumber)
{
	lock_guard<mutex> lock(itemMutex);
	itemMap[itemNumber]->state = EItemState::Deactivated;
	if (activatedItemMap.find(itemNumber) != activatedItemMap.end())
	{
		activatedItemMap.erase(itemNumber);
	}
	deactivatedItemMap[itemNumber] = itemMap[itemNumber];
}

void ItemManager::SaveItemInfoToPacket(std::ostream& stream)
{
	lock_guard<mutex> lock(itemMutex);

	// 아이템의 상태(enum)와 소유 상태인 경우 소유자 id도 전송
	stream << itemMap.size() << "\n";
	for (auto& kv : itemMap)
	{
		stream << kv.first << "\n";
		stream << kv.second;
	}
}


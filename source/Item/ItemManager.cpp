#include "ItemManager.h"
#include "../JsonComponent.h"

using namespace std;

ItemManager::ItemManager()
{
	jsonComponent = make_unique<JsonComponent>();
	jsonComponent->Initialize();
	
	itemMap[0] = make_shared<Item>(EItemState::Activated, jsonComponent->GetItemInfo(0), Vector3D{ 310,-220,40 });
	itemMap[1] = make_shared<Item>(EItemState::Activated, jsonComponent->GetItemInfo(1), Vector3D{ 310,140,40 });
	activatedItemMap[0] = itemMap[0];
	activatedItemMap[1] = itemMap[1];

	//float y = 320;
	//for (int i = 0; i < 5; i++)
	//{
	//	itemMap[i] = Item{ EItemState::Deactivated, EItemMainType::MeleeWeapon, EMeleeWeaponType::Axe, Vector3D{-1010,y,340} };
	//	y -= 100;
	//	deactivatedItemMap[i] = &itemMap[i];
	//}
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
	stream << itemMap.size() << "\n";
	for (auto& kv : itemMap)
	{
		stream << kv.first << "\n";
		stream << kv.second;
	}
}


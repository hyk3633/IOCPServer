#include "ItemManager.h"

using namespace std;

ItemManager::ItemManager()
{
	itemMap[0] = Item{ EItemState::Activated, EItemMainType::MeleeWeapon, EMeleeWeaponType::Axe, Vector3D{275,165,100} };
	activatedItemMap[0] = &itemMap[0];

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
	itemMap[itemNumber].state = EItemState::Activated;
	if (deactivatedItemMap.find(itemNumber) != deactivatedItemMap.end())
	{
		deactivatedItemMap.erase(itemNumber);
	}
	activatedItemMap[itemNumber] = &itemMap[itemNumber];
}

void ItemManager::SetItemStateToDeactivated(const int itemNumber)
{
	lock_guard<mutex> lock(itemMutex);
	itemMap[itemNumber].state = EItemState::Deactivated;
	if (activatedItemMap.find(itemNumber) != activatedItemMap.end())
	{
		activatedItemMap.erase(itemNumber);
	}
	deactivatedItemMap[itemNumber] = &itemMap[itemNumber];
}

void ItemManager::SaveItemInfoToPacket(std::ostream& stream)
{
	lock_guard<mutex> lock(itemMutex);
	stream << itemMap.size() << "\n";
	for (auto& kv : itemMap)
	{
		stream << kv.first << "\n";
		stream << &kv.second;
	}
}


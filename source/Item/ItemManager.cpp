#include "ItemManager.h"

using namespace std;

ItemManager::ItemManager()
{
	itemMap[0] = Item{ EItemState::Activated, EItemMainType::MeleeWeapon, EMeleeWeaponType::Axe, Vector3D{275,165,100} };
	activatedItemMap[0] = &itemMap[0];
}

void ItemManager::SaveItemInfoToPacket(std::ostream& stream)
{
	lock_guard<mutex> lock(itemMutex);
	stream << activatedItemMap.size() << "\n";
	for (auto& kv : activatedItemMap)
	{
		stream << kv.first << "\n";
		stream << kv.second;
	}
}


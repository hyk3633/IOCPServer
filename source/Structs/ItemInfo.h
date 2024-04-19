#pragma once
#include <string>
#include <sstream>
#include "../Structs/IntPoint.h"
#include "../Enums/ItemType.h"

struct ItemInfo
{
	ItemInfo() {};

	int itemKey;

	std::string itemName;

	EItemMainType itemType;

	IntPoint itemGridSize;

	void CopyTo(ItemInfo* otherInfo)
	{
		otherInfo->itemKey = itemKey;
		otherInfo->itemName = itemName;
		otherInfo->itemType = itemType;
		otherInfo->itemGridSize = itemGridSize;
	}
};

struct MeleeWeaponInfo : public ItemInfo
{
	MeleeWeaponInfo() {};

	float attackPower;

	float attackSpeed;

	void CopyTo(MeleeWeaponInfo* otherInfo)
	{
		ItemInfo::CopyTo(otherInfo);
		otherInfo->attackPower = attackPower;
		otherInfo->attackSpeed = attackSpeed;
	}

};

struct RangedWeaponInfo : public ItemInfo
{
	RangedWeaponInfo() {};

	float attackPower;

	float fireRate;

	float recoil;

	int magazine;

	float reloadingSpeed;

	void CopyTo(RangedWeaponInfo* otherInfo)
	{
		ItemInfo::CopyTo(otherInfo);
		otherInfo->attackPower = attackPower;
		otherInfo->fireRate = fireRate;
		otherInfo->recoil = recoil;
		otherInfo->magazine = magazine;
		otherInfo->reloadingSpeed = reloadingSpeed;
	}

};

struct RecoveryItemInfo : public ItemInfo
{
	RecoveryItemInfo() {};

	int recoveryAmount;

	float usingSpeed;

	void CopyTo(RecoveryItemInfo* otherInfo)
	{
		ItemInfo::CopyTo(otherInfo);
		otherInfo->recoveryAmount = recoveryAmount;
		otherInfo->usingSpeed = usingSpeed;
	}

};

struct AmmoItemInfo : public ItemInfo
{
	AmmoItemInfo() {};

	int amount;

	int ammoType;

	void CopyTo(AmmoItemInfo* otherInfo)
	{
		ItemInfo::CopyTo(otherInfo);
		otherInfo->amount = amount;
		otherInfo->ammoType = ammoType;
	}

};
#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "../Structs/GridPoint.h"
#include "../Enums/ItemType.h"
#include "../Enums/WeaponType.h"

using std::ostream;

struct ItemInfo
{
	ItemInfo() {}

	int itemKey;
	std::string itemName;
	EItemMainType itemType;
	GridPoint itemGridSize;
	int count;
	bool isConsumable;
	
};

struct WeaponInfo
{
	WeaponInfo() {}

	float attackPower;
	EWeaponType weaponType;

	friend ostream& operator<<(ostream& stream, WeaponInfo& info)
	{
		stream << info.attackPower << "\n";
		stream << static_cast<int>(info.weaponType) << "\n";
		return stream;
	}

};

struct RangedWeaponInfo : public WeaponInfo
{
	RangedWeaponInfo() {}

	float fireRate;
	float recoil;
	int magazine;
	float reloadingSpeed;

	friend ostream& operator<<(ostream& stream, RangedWeaponInfo& info)
	{
		stream << info.attackPower << "\n";
		stream << static_cast<int>(info.weaponType) << "\n";
		stream << info.fireRate << "\n";
		stream << info.recoil << "\n";
		stream << info.magazine << "\n";
		stream << info.reloadingSpeed << "\n";
		return stream;
	}

};

struct RecoveryItemInfo
{
	RecoveryItemInfo() {}

	int recoveryAmount;
	float usingSpeed;

	friend ostream& operator<<(ostream& stream, RecoveryItemInfo& info)
	{
		stream << info.recoveryAmount << "\n";
		stream << info.usingSpeed << "\n";
		return stream;
	}

};

struct AmmoItemInfo
{
	AmmoItemInfo() {}

	int ammoType;

	friend ostream& operator<<(ostream& stream, AmmoItemInfo& info)
	{
		stream << info.ammoType << "\n";
		return stream;
	}

};
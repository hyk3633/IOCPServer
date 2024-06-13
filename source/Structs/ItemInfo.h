#pragma once
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include "../Structs/GridPoint.h"
#include "../Enums/ItemType.h"
#include "../Enums/WeaponType.h"

using std::ostream;
using std::istream;
using std::stringstream;

template<typename T> 
void ParseInfo(stringstream& stream, T& info)
{
	stringstream temp;
	temp << stream.str();
	temp >> info;
}

struct ItemInfo
{
	ItemInfo() {}

	int itemKey;
	std::string itemName;
	EItemMainType itemType;
	GridPoint itemGridSize;
	int quantity;
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

	friend istream& operator>>(istream& stream, WeaponInfo& info)
	{
		stream >> info.attackPower;
		int type = 0;
		stream >> type;
		info.weaponType = static_cast<EWeaponType>(type);
		return stream;
	}

	static WeaponInfo GetConcreteInfo(const stringstream& concreteInfo)
	{
		stringstream tempInfo;
		tempInfo << concreteInfo.str();

		WeaponInfo info;
		int type = 0;
		tempInfo >> info.attackPower >> type;
		info.weaponType = static_cast<EWeaponType>(type);
		return info;
	}

};

struct RangedWeaponInfo : public WeaponInfo
{
	RangedWeaponInfo() {}

	int magazine;
	int ammoType;

	friend ostream& operator<<(ostream& stream, RangedWeaponInfo& info)
	{
		stream << info.attackPower << "\n";
		stream << static_cast<int>(info.weaponType) << "\n";
		stream << info.ammoType << "\n";
		stream << info.magazine << "\n";
		return stream;
	}

	friend istream& operator>>(istream& stream, RangedWeaponInfo& info)
	{
		stream >> info.attackPower;
		int type = 0;
		stream >> type;
		info.weaponType = static_cast<EWeaponType>(type);
		stream >> info.magazine;
		stream >> info.ammoType;
		return stream;
	}

	static RangedWeaponInfo GetConcreteInfo(const stringstream& concreteInfo)
	{
		stringstream tempInfo;
		tempInfo << concreteInfo.str();

		RangedWeaponInfo info;
		int type = 0;
		tempInfo >> info.attackPower >> type;
		info.weaponType = static_cast<EWeaponType>(type);
		tempInfo >> info.magazine;
		tempInfo >> info.ammoType;
		return info;
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

	friend istream& operator>>(istream& stream, RecoveryItemInfo& info)
	{
		stream >> info.recoveryAmount;
		stream >> info.usingSpeed;
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

	friend istream& operator>>(istream& stream, AmmoItemInfo& info)
	{
		stream >> info.ammoType;
		return stream;
	}

};

struct ArmourItemInfo
{
	ArmourItemInfo() {}

	float defensePower;

	int armourSlot;

	friend ostream& operator<<(ostream& stream, ArmourItemInfo& info)
	{
		stream << info.defensePower << "\n";
		stream << info.armourSlot << "\n";
		return stream;
	}

	friend istream& operator>>(istream& stream, ArmourItemInfo& info)
	{
		stream >> info.defensePower;
		stream >> info.armourSlot;
		return stream;
	}

};
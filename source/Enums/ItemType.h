#pragma once

enum class EItemMainType
{
	MeleeWeapon,
	RangedWeapon,
	RecoveryItem,
	AmmoItem,
	ArmourItem,
	MAX,
};

enum class EItemState
{
	Activated,
	Acquired,
	Deactivated
};
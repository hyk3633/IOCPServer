#pragma once

enum class EItemMainType
{
	MeleeWeapon,
	RangedWeapon,
	RecoveryItem,
	AmmoItem,
	MAX,
};

enum class EItemState
{
	Activated,
	Acquired,
	Deactivated
};
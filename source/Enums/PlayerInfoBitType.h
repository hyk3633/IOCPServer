#pragma once

enum class EPlayerInfoBitTypeClient
{
	ZombiesInRange,
	ZombiesOutRange,
	ZombieAttackResult,
	MAX
};

typedef EPlayerInfoBitTypeClient PIBTC;

enum class EPlayerInfoBitTypeServer
{
	WrestlingState,
	MAX
};

typedef EPlayerInfoBitTypeServer PIBTS;
#pragma once

using namespace std;

#define SERVER_PORT 9999
#define PACKET_SIZE 4096

enum class EPacketType
{
	SIGNUP,
	LOGIN,
	SPAWNPLAYER,
	SYNCHPLAYER,
	ZOMBIEINRANGE,
	ZOMBIEOUTRANGE,
	WRESTLINGRESULT,
	WRESTLINGSTART,
	PICKUP_ITEM,
	ITEMGRIDPOINTUPDATE,
	EQUIP_ITEM,
	DROP_ITEM,
	DROP_EQUIPPED_ITEM,
	UNEQUIP_ITEM,
	ATTACKRESULT,
	KICKEDCHARACTERS,
	ZOMBIEHITSME,
	PLAYERRESPAWN,
	PROJECTILE,
	USINGITEM,
	DESTROYITEM,
	ACTIVATE_WEAPON_ABILITY,
	PACKETTYPE_MAX, // MAX
	WORLDINITIALINFO,
	SYNCHZOMBIE,
	PLAYERDISCONNECTED,
	ZOMBIEDEAD,
	PLAYERDEAD,
	SPAWNITEM,
	PLAYERINVENTORY,
	PLAYEREQUIPMENT,
	HEALTH_CHANGED,
	PLAYERINITIALINFO,
	CHANGE_WEAPON,
	ARM_WEAPON,
	DISARM_WEAPON,
	PLAYER_WRESTLING_CANCELED,
};






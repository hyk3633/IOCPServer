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
	PLAYERINPUTACTION,
	ZOMBIEINRANGE,
	ZOMBIEOUTRANGE,
	WRESTLINGRESULT,
	WRESTLINGSTART,
	SYNCHITEM,			
	HITPLAYER,
	HITZOMBIE,
	ZOMBIEHITSME,
	PLAYERRESPAWN,
	PACKETTYPE_MAX,		// MAX, MAX 다음 부터는 송신 전용
	INITIALINFO,
	SYNCHZOMBIE,		
	PLAYERDISCONNECTED,	
	DESTROYITEM,
	PICKUPITEM,
	ZOMBIEDEAD,
	PLAYERDEAD,
	SPAWNITEM,
};






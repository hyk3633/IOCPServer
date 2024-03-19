#pragma once

using namespace std;

#define SERVER_PORT 9999
#define PACKET_SIZE 4096

typedef void(*WrestlingBroadcast)(int);

enum class EPacketType
{
	SIGNUP,
	LOGIN,
	SPAWNPLAYER,
	SYNCHPLAYER,
	PLAYERINPUTACTION,
	WRESTLINGRESULT,
	WRESTLINGSTART,
	PACKETTYPE_MAX,
	SYNCHZOMBIE,
	PLAYERDISCONNECTED
};






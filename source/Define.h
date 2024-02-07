#pragma once

#define SERVER_PORT 9999
#define PACKET_SIZE 4096

enum class EPacketType
{
	SIGNUP,
	LOGIN,
	PACKETTYPE_MAX
};
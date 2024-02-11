#pragma once
#include "IOCPServer.h"
#include "PlayerInfo.h"
#include <unordered_map>

using namespace std;

class GameServer : public IOCPServer
{
public:

	GameServer();

	virtual ~GameServer();

	virtual bool InitializeServer() override;

protected:

	static void SignUp(SocketInfo*, stringstream&);

	static void Login(SocketInfo*, stringstream&);

	static void SpawnOtherPlayers(SocketInfo*, stringstream&);

	static void SynchronizePlayerInfo(SocketInfo*, stringstream&);

private:

	static CRITICAL_SECTION	critsecPlayerInfo;
	
	static unordered_map<int, SocketInfo*> playerSocketMap;

	static PlayerInfoSetEx playerInfoSetEx;

	static int playerCount;

};
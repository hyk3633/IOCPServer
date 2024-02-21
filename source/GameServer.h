#pragma once
#include "IOCPServer.h"
#include "CharacterInfo.h"
#include <unordered_map>

using namespace std;

class GameServer : public IOCPServer
{
public:

	GameServer();

	virtual ~GameServer();

	virtual bool InitializeServer() override;

	void ZombieThread();

protected:

	virtual bool CreateZombieThread();

	void InitializeZombieInfo();

	virtual void HandleDisconnectedClient(SocketInfo* socketInfo) override;

	static void SignUp(SocketInfo*, stringstream&);

	static void Login(SocketInfo*, stringstream&);

	static void SpawnOtherPlayers(SocketInfo*, stringstream&);

	static void SynchronizePlayerInfo(SocketInfo*, stringstream&);

	static void BroadcastPlyerInputAction(SocketInfo*, stringstream&);

	static void Broadcast(stringstream&, const int skipNumber = -1);

private:

	static CRITICAL_SECTION	critsecPlayerInfo;
	
	static unordered_map<int, SocketInfo*> playerSocketMap;

	static PlayerInfoSetEx playerInfoSetEx;

	static CharacterInfoSet zombieInfoSet;

	HANDLE* zombieThread;

	const int maxZombieCount = 3;

	static int playerCount;

};
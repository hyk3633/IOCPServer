#pragma once
#include "IOCPServer.h"
#include "CharacterInfo.h"
#include "Zombie/Zombie.h"
#include "Pathfinder/PathFinder.h"
#include <unordered_map>

using namespace std;

class GameServer : public IOCPServer
{
public:

	GameServer();

	virtual ~GameServer();

	virtual bool InitializeServer() override;

	void ZombieThread();

	void SyncThread();

protected:

	virtual bool CreateZombieThread();

	void InitializeZombieInfo();

	virtual void HandleDisconnectedClient(SocketInfo* socketInfo) override;

	static void SignUp(SocketInfo*, stringstream&);

	static void Login(SocketInfo*, stringstream&);

	static void SpawnOtherPlayers(SocketInfo*, stringstream&);

	static void SaveZombieInfoToPacket(stringstream&);

	static void SynchronizePlayerInfo(SocketInfo*, stringstream&);

	static void ProcessPlayerInfo(const int playerNumber, PlayerInfo& info);

	static void CheckInfoBitAndProcess(const int playerNumber, PlayerInfo& info, const PIBTC bitType);

	static void BroadcastPlyerInputAction(SocketInfo*, stringstream&);

	static void ProcessPlayerWrestlingResult(SocketInfo*, stringstream&);

	static void ProcessPlayerWrestlingStart(const int playerNumber);

	static void Broadcast(stringstream&, const int skipNumber = -1);

private:

	static CRITICAL_SECTION	critsecPlayerInfo;
	
	static unordered_map<int, SocketInfo*> playerSocketMap;

	static PlayerInfoSetEx playerInfoSetEx;

	static unordered_map<int, Zombie> zombieMap;

	static ZombieInfoSet zombieInfoSet;

	Pathfinder pathfinder;

	HANDLE* zombieThread;

	const int maxZombieCount = 3;

	static int playerCount;

	float zombieThreadElapsedTime;

	float zombiePacketSendingInterval = 0.1f;

	bool packetFlag = true;

};
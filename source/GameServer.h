#pragma once
#include "IOCPServer.h"
#include "Enums/PlayerInfoBitType.h"
#include <unordered_map>
#include <memory>

class ItemManager;
class Zombie;
class Player;

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

	static void SignUp(SocketInfo*, std::stringstream&);

	static void Login(SocketInfo*, std::stringstream&);

	static void SpawnOtherPlayers(SocketInfo*, std::stringstream&);

	static void SaveZombieInfoToPacket(std::stringstream&);

	static void SaveItemInfoToPacket(std::stringstream&);

	static void SynchronizePlayerInfo(SocketInfo*, std::stringstream&);

	static void ProcessPlayerInfo(std::shared_ptr<Player> player);

	static void CheckInfoBitAndProcess(std::shared_ptr<Player> player, const PIBTC bitType);

	static void BroadcastPlayerInputAction(SocketInfo*, std::stringstream&);

	static void ProcessPlayerWrestlingResult(SocketInfo*, std::stringstream&);

	static void ProcessPlayerWrestlingStart(const int playerNumber);

	static void SynchronizeItemInfo(SocketInfo*, std::stringstream&);

	static void Broadcast(std::stringstream&, const int skipNumber = -1);

private:

	static CRITICAL_SECTION	critsecPlayerInfo;
	
	static std::unordered_map<int, SocketInfo*> playerSocketMap;

	static std::unique_ptr<ItemManager> itemManager;

	static std::unordered_map<int, std::string> playerIDMap;

	static std::unordered_map<int, std::shared_ptr<Player>> playerMap;

	static std::unordered_map<int, std::shared_ptr<Zombie>> zombieMap;

	HANDLE* zombieThread;

	const int maxZombieCount = 3;

	static int playerCount;

	float zombieThreadElapsedTime;

	float zombiePacketSendingInterval = 0.1f;

	bool packetFlag = true;

};
#pragma once
#include "IOCPServer.h"
#include <unordered_map>
#include <memory>

class ZombieManager;
class ItemManager;
class JsonComponent;
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

	void RemovePlayerInfo(const int playerNumber);

	static void SignUp(SocketInfo*, std::stringstream&);

	static void Login(SocketInfo*, std::stringstream&);

	static void SpawnOtherPlayers(SocketInfo*, std::stringstream&);

	static void SaveZombieInfoToPacket(std::stringstream&);

	static void SaveItemInfoToPacket(std::stringstream&);

	static void SynchronizePlayerInfo(SocketInfo*, std::stringstream&);

	static void BroadcastPlayerInputAction(SocketInfo*, std::stringstream&);

	static void ProcessInRangeZombie(SocketInfo*, std::stringstream&);

	static void ProcessOutRangeZombie(SocketInfo*, std::stringstream&);

	static void ProcessPlayerWrestlingResult(SocketInfo*, std::stringstream&);

	static void ProcessPlayerWrestlingStart(const int playerNumber);

	static void ProcessZombieHitResult(SocketInfo*, std::stringstream&);

	static void PlayerItemPickUp(SocketInfo*, std::stringstream&);

	static void PlayerItemGridPointUpdate(SocketInfo*, std::stringstream&);

	static void PlayerItemEquip(SocketInfo*, std::stringstream&);

	static void PlayerItemDrop(SocketInfo*, std::stringstream&);

	static void Broadcast(std::stringstream&, const int skipNumber = -1);

	static void ProcessPlayerAttackResult(SocketInfo*, std::stringstream&);

	static void ProcessPlayerDead(const int playerNumber);

	static void ProcessZombieDead(const int zombieNumber);

	static void RespawnPlayer(SocketInfo*, std::stringstream&);

private:

	static CRITICAL_SECTION	critsecPlayerInfo;

	static CRITICAL_SECTION	critsecZombieInfo;
	
	static std::unordered_map<int, SocketInfo*> playerSocketMap;

	static std::unique_ptr<ItemManager> itemManager;

	static std::unordered_map<int, std::string> playerIDMap;

	static std::unordered_map<int, std::shared_ptr<Player>> playerMap;

	static std::unique_ptr<ZombieManager> zombieManager;

	static std::unordered_map<int, std::shared_ptr<Zombie>> zombieMap;

	HANDLE* zombieThread;

	const int maxZombieCount = 3;

	static int playerCount;

	float zombieThreadElapsedTime;

	float zombiePacketSendingInterval = 0.1f;

};
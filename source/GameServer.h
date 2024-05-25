#pragma once
#include "IOCPServer.h"
#include <unordered_map>
#include <memory>
#include "Structs/PossessedItem.h"
#include "Structs/EquippedItem.h"
#include "Structs/PlayerInfo.h"

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

	void SavePlayerInfo(const int playerNumber);

	void RemovePlayerInfo(const int playerNumber);

	static void SignUp(SocketInfo*, std::stringstream&);

	static void Login(SocketInfo*, std::stringstream&);

	static void NewPlayerAccessToGameMap(SocketInfo*, std::stringstream&);

	static void SerializeOthersToNewPlayer(const int playerNumber, std::stringstream&);

	static void SerializePlayersEquippedItems(shared_ptr<Player> player, std::stringstream&);

	static void SerializeNewPlayerToOthers(shared_ptr<Player> player, const int playerNumber, std::stringstream&);

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

	static void PlayerUnequipItem(SocketInfo*, std::stringstream&);

	static void PlayerItemDrop(SocketInfo*, std::stringstream&);

	static void PlayerDropEquippedItem(SocketInfo*, std::stringstream&);

	static void Broadcast(std::stringstream&, const int skipNumber = -1);

	static void ProcessPlayerAttackResult(SocketInfo*, std::stringstream&);

	static void ProcessPlayerDead(const int playerNumber);

	static void ProcessZombieDead(const int zombieNumber);

	static void RespawnPlayer(SocketInfo*, std::stringstream&);

	static void ReplicateProjectile(SocketInfo*, std::stringstream&);

	static void PlayerUseItem(SocketInfo*, std::stringstream&);

	static void DestroyItem(const int playerNumber, shared_ptr<Item> item, const string& itemID);

	static void PlayerChangedWeapon(SocketInfo*, std::stringstream&);

	static void PlayerArmWeapon(SocketInfo*, std::stringstream&);

	static void PlayerDisarmWeapon(SocketInfo*, std::stringstream&);

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

	static PlayerInfo playerInfo;

};
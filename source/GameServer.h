#pragma once
#include "IOCPServer.h"
#include <unordered_map>
#include <memory>
#include "Structs/PossessedItem.h"
#include "Structs/EquippedItem.h"
#include "Structs/PlayerInfo.h"
#include "Structs/HitInfo.h"
#include "Structs/Pos.h"

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

	// ������ �Ҹ� �� ȣ��Ǵ� �ݹ� �Լ�
	static void DestroyItem(const int playerNumber, shared_ptr<Item> item, const string& itemID);

	/* ���� ������ */

	void ZombieThread();

protected:

	virtual bool CreateZombieThread();

	void InitializeZombieInfo();

	// ���� ��� �ݹ� �Լ�
	static void ProcessZombieDead(const int zombieNumber);

	// ĳ���� ü�� ���� �ݹ� �Լ�
	static void CharacterHealthChanged(const int number, const float health, const bool isPlayer);


	/* �÷��̾� ���� ó�� */

	virtual void HandleDisconnectedClient(SocketInfo* socketInfo) override;

	void SavePlayerInfo(const int playerNumber);

	void RemovePlayerInfo(const int playerNumber);

	/* �α��� */

	static void SignUp(SocketInfo*, std::stringstream&);

	static void Login(SocketInfo*, std::stringstream&);

	/* �÷��̾� ����ȭ */

	static void SpawnNewPlayerInGameMap(SocketInfo*, std::stringstream&);

	// ���� ���� ����ȭ
	static void SaveZombieInfoToPacket(std::stringstream&);

	// ������ ���� ����ȭ
	static void SaveItemInfoToPacket(std::stringstream&);

	// �ٸ� �÷��̾���� ������ ���� ������ �÷��̾�� ����
	static void SerializeOthersToNewPlayer(const int playerNumber, std::stringstream&);

	// �÷��̾��� ���� ������ ���� ����ȭ
	static void SerializePlayersEquippedItems(shared_ptr<Player> player, std::stringstream&);

	// ���� ������ �÷��̾��� ������ �ٸ� �÷��̾�鿡�� ����
	static void SerializeNewPlayerToOthers(shared_ptr<Player> player, const int playerNumber, std::stringstream&);

	// �÷��̾� ��� �� ȣ��Ǵ� �ݹ� �Լ�
	static void ProcessPlayerDead(const int playerNumber);

	static void SynchronizePlayerInfo(SocketInfo*, std::stringstream&);

	static void RespawnPlayer(SocketInfo*, std::stringstream&);

	/* ������ ���� ����ȭ */

	// �÷��̾� ������ ���� �ݹ� �Լ�
	static void ProcessPlayerWrestlingStart(const int playerNumber);
	
	static void ProcessPlayerWrestlingResult(SocketInfo*, std::stringstream&);

	/* �÷��̾� ���� �� ���� ���� */

	static void ProcessInRangeZombie(SocketInfo*, std::stringstream&);

	static void ProcessOutRangeZombie(SocketInfo*, std::stringstream&);

	/* ���� ���� ��� ó�� */

	static void ProcessZombieHitResult(SocketInfo*, std::stringstream&);

	/* �κ��丮 ������ �׸��� ��ġ ���� */

	static void UpdatePlayerItemGridPoint(SocketInfo*, std::stringstream&);

	/* �÷��̾� ���� ����ȭ */

	static void ActivateWeaponAbility(SocketInfo*, std::stringstream&);

	static void ProcessPlayerAttackResult(SocketInfo*, std::stringstream&);

	static void ProcessKickedCharacters(SocketInfo*, std::stringstream&);

	// �÷��̾� ������ ��� �ݹ� �Լ�
	static void CancelPlayerWrestling(std::weak_ptr<Player> playerPtr);

	static void ReplicateProjectile(SocketInfo*, std::stringstream&);

	/* ������ ����ȭ */

	static void SynchronizeItemPickingUp(SocketInfo*, std::stringstream&);

	static void SynchronizeItemDropping(SocketInfo*, std::stringstream&);

	static void SynchronizeItemEquipping(SocketInfo*, std::stringstream&);

	static void SynchronizeItemUnequipping(SocketInfo*, std::stringstream&);

	static void SynchronizeEquippedItemDropping(SocketInfo*, std::stringstream&);

	static void SynchronizeWeaponArming(SocketInfo*, std::stringstream&);

	static void SynchronizeWeaponDisarming(SocketInfo*, std::stringstream&);

	static void SynchronizeWeaponChanging(SocketInfo*, std::stringstream&);

	static void SynchronizeItemUsing(SocketInfo*, std::stringstream&);

	static void Broadcast(std::stringstream&, const int skipNumber = -1);

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
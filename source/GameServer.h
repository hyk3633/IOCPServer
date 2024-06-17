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

	// 아이템 소모 시 호출되는 콜백 함수
	static void DestroyItem(const int playerNumber, shared_ptr<Item> item, const string& itemID);

	/* 좀비 스레드 */

	void ZombieThread();

protected:

	virtual bool CreateZombieThread();

	void InitializeZombieInfo();

	// 좀비 사망 콜백 함수
	static void ProcessZombieDead(const int zombieNumber);

	// 캐릭터 체력 변경 콜백 함수
	static void CharacterHealthChanged(const int number, const float health, const bool isPlayer);


	/* 플레이어 정보 처리 */

	virtual void HandleDisconnectedClient(SocketInfo* socketInfo) override;

	void SavePlayerInfo(const int playerNumber);

	void RemovePlayerInfo(const int playerNumber);

	/* 로그인 */

	static void SignUp(SocketInfo*, std::stringstream&);

	static void Login(SocketInfo*, std::stringstream&);

	/* 플레이어 동기화 */

	static void SpawnNewPlayerInGameMap(SocketInfo*, std::stringstream&);

	// 좀비 정보 직렬화
	static void SaveZombieInfoToPacket(std::stringstream&);

	// 아이템 정보 직렬화
	static void SaveItemInfoToPacket(std::stringstream&);

	// 다른 플레이어들의 정보를 새로 접속한 플레이어에게 전송
	static void SerializeOthersToNewPlayer(const int playerNumber, std::stringstream&);

	// 플레이어의 장착 아이템 정보 직렬화
	static void SerializePlayersEquippedItems(shared_ptr<Player> player, std::stringstream&);

	// 새로 접속한 플레이어의 정보를 다른 플레이어들에게 전송
	static void SerializeNewPlayerToOthers(shared_ptr<Player> player, const int playerNumber, std::stringstream&);

	// 플레이어 사망 시 호출되는 콜백 함수
	static void ProcessPlayerDead(const int playerNumber);

	static void SynchronizePlayerInfo(SocketInfo*, std::stringstream&);

	static void RespawnPlayer(SocketInfo*, std::stringstream&);

	/* 레슬링 상태 동기화 */

	// 플레이어 레슬링 시작 콜백 함수
	static void ProcessPlayerWrestlingStart(const int playerNumber);
	
	static void ProcessPlayerWrestlingResult(SocketInfo*, std::stringstream&);

	/* 플레이어 범위 내 좀비 감지 */

	static void ProcessInRangeZombie(SocketInfo*, std::stringstream&);

	static void ProcessOutRangeZombie(SocketInfo*, std::stringstream&);

	/* 좀비 공격 결과 처리 */

	static void ProcessZombieHitResult(SocketInfo*, std::stringstream&);

	/* 인벤토리 아이템 그리드 위치 변경 */

	static void UpdatePlayerItemGridPoint(SocketInfo*, std::stringstream&);

	/* 플레이어 공격 동기화 */

	static void ActivateWeaponAbility(SocketInfo*, std::stringstream&);

	static void ProcessPlayerAttackResult(SocketInfo*, std::stringstream&);

	static void ProcessKickedCharacters(SocketInfo*, std::stringstream&);

	// 플레이어 레슬링 취소 콜백 함수
	static void CancelPlayerWrestling(std::weak_ptr<Player> playerPtr);

	static void ReplicateProjectile(SocketInfo*, std::stringstream&);

	/* 아이템 동기화 */

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
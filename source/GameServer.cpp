#include "GameServer.h"
#include "JsonComponent.h"
#include "Item/ItemManager.h"
#include "Structs/ItemInfo.h"
#include "Player/Player.h"
#include "Zombie/ZombieManager.h"
#include "Zombie/Zombie.h"
#include "Zombie/State/IdleState.h"
#include "Zombie/State/WaitState.h"
#include "Pathfinder/PathFinder.h"
#include <sstream>
#include <chrono>
#include <iostream>
using namespace std;

CRITICAL_SECTION						GameServer::critsecPlayerInfo;
CRITICAL_SECTION						GameServer::critsecZombieInfo;
unordered_map<int, SocketInfo*>			GameServer::playerSocketMap;
unique_ptr<ItemManager>					GameServer::itemManager;
unordered_map<int, string>				GameServer::playerIDMap;
unordered_map<int, shared_ptr<Player>>	GameServer::playerMap;
int										GameServer::playerCount;
unique_ptr<ZombieManager>				GameServer::zombieManager;
unordered_map<int, shared_ptr<Zombie>>	GameServer::zombieMap;
PlayerInfo								GameServer::playerInfo;

unsigned int WINAPI ZombieThreadStart(LPVOID param)
{
	GameServer* gameEvent = reinterpret_cast<GameServer*>(param);
	gameEvent->ZombieThread();
	return 0;
}

GameServer::GameServer()
{

}

GameServer::~GameServer()
{

}

bool GameServer::InitializeServer()
{
	bool result = IOCPServer::InitializeServer();
	if (!result) return result;

	itemManager = make_unique<ItemManager>(DestroyItem);

	itemManager->GetPlayerInfo(playerInfo);

	// 로그인
	packetCallbacks[EPacketType::SIGNUP]							= SignUp;
	packetCallbacks[EPacketType::LOGIN]								= Login;

	// 플레이어 동기화
	packetCallbacks[EPacketType::SPAWN_PLAYER]						= SpawnNewPlayerInGameMap;
	packetCallbacks[EPacketType::SYNCH_PLAYER]						= SynchronizePlayerInfo;
	packetCallbacks[EPacketType::PLAYER_RESPAWN]					= RespawnPlayer;

	// 플레이어 좀비 레슬링
	packetCallbacks[EPacketType::WRESTLING_RESULT]					= ProcessPlayerWrestlingResult;

	// 플레이어 Sphere Collision에 좀비가 오버랩 되었을 경우 (좀비의 플레이어 감지)
	packetCallbacks[EPacketType::ZOMBIE_IN_RANGE]					= ProcessInRangeZombie;
	packetCallbacks[EPacketType::ZOMBIE_OUT_RANGE]					= ProcessOutRangeZombie;

	// 좀비의 플레이어 공격  
	packetCallbacks[EPacketType::ZOMBIE_HITS_ME]					= ProcessZombieHitResult;

	// 플레이어의 인벤토리의 아이템 그리드 위치 변경
	packetCallbacks[EPacketType::ITEM_GRID_POINT_UPDATE]			= UpdatePlayerItemGridPoint;

	// 플레이어의 공격 동기화
	packetCallbacks[EPacketType::ACTIVATE_WEAPON_ABILITY]			= ActivateWeaponAbility;
	packetCallbacks[EPacketType::ATTACK_RESULT]						= ProcessPlayerAttackResult;
	packetCallbacks[EPacketType::KICKED_CHARACTERS]					= ProcessKickedCharacters;
	packetCallbacks[EPacketType::PROJECTILE]						= ReplicateProjectile;
	
	// 아이템 동기화
	packetCallbacks[EPacketType::PICKUP_ITEM]						= SynchronizeItemPickingUp;
	packetCallbacks[EPacketType::DROP_ITEM]							= SynchronizeItemDropping;
	packetCallbacks[EPacketType::EQUIP_ITEM]						= SynchronizeItemEquipping;
	packetCallbacks[EPacketType::UNEQUIP_ITEM]						= SynchronizeItemUnequipping;
	packetCallbacks[EPacketType::DROP_EQUIPPED_ITEM]				= SynchronizeEquippedItemDropping;
	packetCallbacks[EPacketType::ARM_WEAPON]						= SynchronizeWeaponArming;
	packetCallbacks[EPacketType::DISARM_WEAPON]						= SynchronizeWeaponDisarming;
	packetCallbacks[EPacketType::CHANGE_WEAPON]						= SynchronizeWeaponChanging;
	packetCallbacks[EPacketType::USING_ITEM]						= SynchronizeItemUsing;

	InitializeCriticalSection(&critsecPlayerInfo);

	return true;
}

void GameServer::DestroyItem(const int playerNumber, shared_ptr<Item> item, const string& itemID)
{
	// 아이템 파괴
	playerMap[playerNumber]->RemoveItemInInventory(item, itemID);
}

void GameServer::ZombieThread()
{
	InitializeZombieInfo();
	
	bool packetFlag = true, sendFlag = false;
	int zombieCount = 0;

	while (1)
	{
		if (zombieThreadElapsedTime >= zombiePacketSendingInterval)
		{
			packetFlag = true;
		}
		if (playerMap.size())
		{
			EnterCriticalSection(&critsecZombieInfo);
			if (zombieMap.size() == 0)
			{
				break;
			}

			for (auto& kv : zombieMap)
			{
				kv.second->Update();
				if (kv.second->GetSendInfoBit())
				{
					zombieCount++;
					sendFlag = true;
				}
			}

			if (packetFlag && sendFlag)
			{
				stringstream sendStream;
				sendStream << static_cast<int>(EPacketType::SYNCH_ZOMBIE) << "\n";
				sendStream << zombieCount << "\n";
				for (auto& kv : zombieMap)
				{
					kv.second->SerializeData(sendStream);
				}

				EnterCriticalSection(&critsecPlayerInfo);
				Broadcast(sendStream);
				LeaveCriticalSection(&critsecPlayerInfo);
				packetFlag = false;
				sendFlag = false;
				zombieThreadElapsedTime = 0;
			}
			zombieCount = 0;
			LeaveCriticalSection(&critsecZombieInfo);
		}
		zombieThreadElapsedTime += 0.008f;
		Sleep(8);
	}
}

bool GameServer::CreateZombieThread()
{
	unsigned int threadId;
	zombieThread = (HANDLE*)_beginthreadex(NULL, 0, &ZombieThreadStart, this, CREATE_SUSPENDED, &threadId);
	if (zombieThread == NULL) return false;
	ResumeThread(zombieThread);
	cout << "[Log] : Start zombie thread!\n";
	return true;
}

void GameServer::InitializeZombieInfo()
{
	InitializeCriticalSection(&critsecZombieInfo);

	zombieManager = make_unique<ZombieManager>();

	zombieManager->InitZombies(zombieMap);

	for (auto& zombie : zombieMap)
	{
		zombie.second->RegisterZombieDeadCallback(ProcessZombieDead);
		zombie.second->RegisterZombieHealthChangedCallback(CharacterHealthChanged);
	}
}

void GameServer::ProcessZombieDead(const int zombieNumber)
{
	cout << "[Log] : 좀비 " << zombieNumber << "가 죽었습니다.\n";

	EnterCriticalSection(&critsecZombieInfo);
	shared_ptr<Zombie> zombie = zombieMap[zombieNumber];
	zombieMap.erase(zombieNumber);
	LeaveCriticalSection(&critsecZombieInfo);

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::ZOMBIE_DEAD) << "\n";
	sendStream << zombieNumber << "\n";
	Broadcast(sendStream);
}

void GameServer::CharacterHealthChanged(const int number, const float health, const bool isPlayer)
{
	stringstream healthChangedStream;

	healthChangedStream << static_cast<int>(EPacketType::HEALTH_CHANGED) << "\n";
	healthChangedStream << number << "\n";
	healthChangedStream << health << "\n";
	healthChangedStream << isPlayer << "\n";

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(healthChangedStream);
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::HandleDisconnectedClient(SocketInfo* socketInfo)
{
	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::PLAYER_DISCONNECTED) << "\n";
	sendStream << socketInfo->number << "\n";

	cout << "[Log] : " << socketInfo->number << "번 클라이언트 (ID : " << playerIDMap[socketInfo->number]  << ") 접속 종료\n";

	if (playerMap.find(socketInfo->number) != playerMap.end())
	{
		SavePlayerInfo(socketInfo->number);
	}
	RemovePlayerInfo(socketInfo->number);

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(sendStream, socketInfo->number);
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::SavePlayerInfo(const int playerNumber)
{
	const string& playerID = playerIDMap[playerNumber];

	dbConnector->DeleteAllPlayerInventory(playerID);
	const auto& possessedItems = playerMap[playerNumber]->GetPossessedItems();
	for (auto& itemPair : possessedItems)
	{
		auto item = itemManager->GetItem(itemPair.first);
		dbConnector->SavePlayerInventory(playerID, itemPair.first, item->itemInfo.itemKey, item->itemInfo.quantity, item->isRotated, itemPair.second);
		itemManager->RemoveItem(itemPair.first);
	}
	
	dbConnector->DeleteAllPlayerEquipment(playerID);
	const auto& equipmentItems = playerMap[playerNumber]->GetEquippedItems();
	for (auto& itemPair : equipmentItems)
	{
		auto item = itemManager->GetItem(itemPair.first);
		dbConnector->SavePlayerEquipment(playerID, itemPair.first, item->itemInfo.itemKey, itemPair.second);
		itemManager->RemoveItem(itemPair.first);
	}
}

void GameServer::RemovePlayerInfo(const int playerNumber)
{
	playerSocketMap.erase(playerNumber);
	playerIDMap.erase(playerNumber);
	playerMap.erase(playerNumber);
}

void GameServer::SignUp(SocketInfo* socketInfo, stringstream& recvStream)
{
	string id, pw;
	recvStream >> id >> pw;
	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::SIGNUP) << "\n";
	const bool result = dbConnector->PlayerSignUp(id, pw);
	sendStream << result << "\n";
	Send(socketInfo, sendStream);
}

void GameServer::Login(SocketInfo* socketInfo, stringstream& recvStream)
{
	string id, pw;
	recvStream >> id >> pw;
	const bool isLoginSuccess = dbConnector->PlayerLogin(id, pw);

	EnterCriticalSection(&critsecPlayerInfo);
	if (isLoginSuccess && socketInfo->number == -1)
	{
		playerIDMap[playerCount] = id;
		playerSocketMap[playerCount] = socketInfo;
		socketInfo->number = playerCount;
		playerCount++;
	}
	LeaveCriticalSection(&critsecPlayerInfo);

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::LOGIN) << "\n";
	sendStream << isLoginSuccess << "\n";
	if (isLoginSuccess)
	{
		sendStream << id << "\n";
		sendStream << socketInfo->number << "\n";
	}
	Send(socketInfo, sendStream);

	cout << "[Log] : ID " << id << " 로그인 요청 -> 결과(" << isLoginSuccess << ")\n";
}

void GameServer::SpawnNewPlayerInGameMap(SocketInfo* socketInfo, stringstream& recvStream)
{
	cout << "[Log] : " << socketInfo->number << "번 클라이언트 게임 맵에 접속\n";

	// 접속한 플레이어의 인벤토리 데이터 DB에서 불러오기
	vector<PossessedItem> possessedItems;
	const bool inventoryResult = dbConnector->GetPlayerInventory(playerIDMap[socketInfo->number], possessedItems);
	if (inventoryResult)
	{
		itemManager->MakePlayersPossessedItems(possessedItems);
	}

	vector<EquippedItem> equippedItems;
	const bool equipmentResult = dbConnector->GetPlayerEquipment(playerIDMap[socketInfo->number], equippedItems);

	if (equipmentResult)
	{
		itemManager->MakePlayersEquippedItems(equippedItems);
	}

	EnterCriticalSection(&critsecPlayerInfo);

	// 플레이어 객체 생성
	playerMap[socketInfo->number] = make_shared<Player>(socketInfo->number, playerInfo);

	auto player = playerMap[socketInfo->number];
	player->PlayerInGameMap();

	// 클라이언트에 보낼 패킷 생성
	stringstream initialInfoStream;
	initialInfoStream << static_cast<int>(EPacketType::WORLD_INITIAL_INFO) << "\n";

	initialInfoStream << static_cast<int>(EPacketType::PLAYER_INITIAL_INFO) << "\n";
	player->SerializePlayerInitialInfo(initialInfoStream);

	// 좀비 데이터 직렬화
	SaveZombieInfoToPacket(initialInfoStream);	

	// 필드에 있는 아이템 데이터 직렬화
	SaveItemInfoToPacket(initialInfoStream);

	// 인벤토리 데이터 직렬화
	if (inventoryResult)
	{
		initialInfoStream << static_cast<int>(EPacketType::PLAYER_INVENTORY) << "\n";
		initialInfoStream << possessedItems.size() << "\n";
		for (auto& possessed : possessedItems)
		{
			auto item = itemManager->GetItem(possessed.itemID);
			player->AddItem({ possessed.topLeftX,possessed.topLeftY }, item->itemInfo.itemGridSize, possessed.itemID);
			initialInfoStream << possessed;
		}
	}
	if (equipmentResult)
	{
		initialInfoStream << static_cast<int>(EPacketType::PLAYER_EQUIPMENT) << "\n";
		initialInfoStream << equippedItems.size() << "\n";
		for (auto& equipped : equippedItems)
		{
			player->ItemEquipInitialize(equipped.itemID, equipped.slotNumber);
			initialInfoStream << equipped;
		}
	}

	// 다른 플레이어들의 데이터 직렬화
	SerializeOthersToNewPlayer(socketInfo->number, initialInfoStream);

	Send(socketInfo, initialInfoStream);

	// 기존의 플레이어들에게 방금 접속한 플레이어 데이터 전송
	SerializeNewPlayerToOthers(player, socketInfo->number, recvStream);

	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::SaveZombieInfoToPacket(stringstream& sendStream)
{
	sendStream << static_cast<int>(EPacketType::SYNCH_ZOMBIE) << "\n";
	sendStream << zombieMap.size() << "\n";
	for (auto& kv : zombieMap)
	{
		kv.second->AllZombieInfoBitOn();
		kv.second->SerializeData(sendStream);
	}
}

void GameServer::SaveItemInfoToPacket(std::stringstream& sendStream)
{
	sendStream << static_cast<int>(EPacketType::SPAWN_ITEM) << "\n";
	itemManager->SaveItemInfoToPacket(sendStream);
}

void GameServer::SerializeOthersToNewPlayer(const int playerNumber, std::stringstream& sendStream)
{
	stringstream otherPlayersStream;
	int count = 0;
	for (auto& p : playerMap)
	{
		if (p.first == playerNumber)
			continue;

		otherPlayersStream << p.first << "\n";					// 플레이어 번호
		otherPlayersStream << playerIDMap[p.first] << "\n";		// 플레이어 아이디
		p.second->SerializeData(otherPlayersStream);			// 플레이어 데이터
		
		// 플레이어 스테이터스 : 체력
		auto status = p.second->GetPlayerStatus();
		otherPlayersStream << status;

		// 장착 중인 아이템
		SerializePlayersEquippedItems(p.second, otherPlayersStream);

		count++;
	}
	if (count)
	{
		sendStream << static_cast<int>(EPacketType::SPAWN_PLAYER) << "\n";
		sendStream << count << "\n"; // 플레이어 수
		sendStream << otherPlayersStream.str() << "\n";
	}
}

void GameServer::SerializePlayersEquippedItems(shared_ptr<Player> player, std::stringstream& sendStream)
{
	auto& equippedItems = player->GetEquippedItems();
	const string armedWeaponID = player->GetArmedWeaponID();

	sendStream << equippedItems.size() << "\n";
	for (auto& equippedItem : equippedItems)
	{
		auto item = itemManager->GetItem(equippedItem.first);
		sendStream << equippedItem.first << "\n";
		sendStream << item->itemInfo.itemKey << "\n";
		sendStream << equippedItem.second << "\n";
		sendStream << (equippedItem.first == armedWeaponID) << "\n";
	}
}

void GameServer::SerializeNewPlayerToOthers(shared_ptr<Player> player, const int playerNumber, std::stringstream& recvStream)
{
	// 콜백 함수 등록
	player->RegisterWrestlingCallback(ProcessPlayerWrestlingStart);
	player->RegisterPlayerDeadCallback(ProcessPlayerDead);
	player->RegisterPlayerHealthChangedCallback(CharacterHealthChanged);

	// 방금 접속한 플레이어의 데이터를 역직렬화
	player->DeserializeData(recvStream);

	// 방금 접속한 플레이어의 데이터 직렬화
	stringstream newPlayerInfoStream;
	newPlayerInfoStream << static_cast<int>(EPacketType::SPAWN_PLAYER) << "\n";
	newPlayerInfoStream << 1 << "\n";
	newPlayerInfoStream << playerNumber << "\n";
	newPlayerInfoStream << playerIDMap[playerNumber] << "\n";
	player->SerializeData(newPlayerInfoStream);

	// 플레이어 스테이터스 : 체력
	auto status = player->GetPlayerStatus();
	newPlayerInfoStream << status;

	// 장착 중인 아이템
	SerializePlayersEquippedItems(player, newPlayerInfoStream);

	Broadcast(newPlayerInfoStream, playerNumber);
}

void GameServer::ProcessPlayerDead(const int playerNumber)
{
	cout << "[Log] : 플레이어 " << playerNumber << "가 죽었습니다.\n";

	EnterCriticalSection(&critsecPlayerInfo);

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::PLAYER_DEAD) << "\n";
	sendStream << playerNumber << "\n";
	Broadcast(sendStream);

	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::SynchronizePlayerInfo(SocketInfo* socketInfo, stringstream& recvStream)
{
	EnterCriticalSection(&critsecPlayerInfo);

	const int clientNumber = socketInfo->number;
	if (playerMap.find(clientNumber) == playerMap.end())
	{
		LeaveCriticalSection(&critsecPlayerInfo);
		return;
	}

	playerMap[clientNumber]->DeserializeData(recvStream);
	playerMap[clientNumber]->Waiting();
	
	int count = 0;
	stringstream sendStream, dataStream;
	for (auto& kv : playerMap)
	{
		if (kv.first == clientNumber || kv.second->GetIsDead())
			continue;
		dataStream << kv.first << "\n";
		kv.second->SerializeData(dataStream);
		count++;
	}
	sendStream << static_cast<int>(EPacketType::SYNCH_PLAYER) << "\n";
	sendStream << count << "\n";
	sendStream << dataStream.str();

	LeaveCriticalSection(&critsecPlayerInfo);

	Send(socketInfo, sendStream);
}

void GameServer::RespawnPlayer(SocketInfo* socketInfo, stringstream& recvStream)
{
	if (playerMap.find(socketInfo->number) != playerMap.end())
	{
		shared_ptr<Player> player = playerMap[socketInfo->number];

		//EnterCriticalSection(&critsecZombieInfo);
		const int zombieNumber = player->GetZombieNumberWrestleWith();
		if (zombieMap.find(zombieNumber) != zombieMap.end())
		{
			zombieMap[zombieNumber]->CheckTargetAndCancelTargetting(socketInfo->number);
		}
		//LeaveCriticalSection(&critsecZombieInfo);

		playerMap[socketInfo->number]->InitializePlayerInfo();

		// 만약 무기 장착 상태로 죽었다면 장착해제 상태로 리스폰
		stringstream sendStream;
		sendStream << static_cast<int>(EPacketType::PLAYER_RESPAWN) << "\n";
		sendStream << socketInfo->number << "\n";
		playerMap[socketInfo->number]->SerializeData(sendStream);
		Broadcast(sendStream);
	}
}

void GameServer::ProcessPlayerWrestlingStart(const int playerNumber)
{
	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::WRESTLING_START) << "\n";
	sendStream << playerNumber << "\n";

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(sendStream);
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::ProcessPlayerWrestlingResult(SocketInfo* socketInfo, stringstream& recvStream)
{
	bool wrestlingResult;
	recvStream >> wrestlingResult;

	shared_ptr<Player> player = playerMap[socketInfo->number];
	player->WrestlStateOff();
	player->SetSuccessToBlocking(wrestlingResult);

	zombieMap[player->GetZombieNumberWrestleWith()]->ChangeState();

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::WRESTLING_RESULT) << "\n";
	sendStream << socketInfo->number << "\n";
	sendStream << wrestlingResult << "\n";

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(sendStream);
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::ProcessInRangeZombie(SocketInfo* socketInfo, stringstream& recvStream)
{
	int playerNumber = socketInfo->number, zombieNumber = -1;
	recvStream >> zombieNumber;

	if (zombieMap.find(zombieNumber) != zombieMap.end() && playerMap.find(playerNumber) != playerMap.end())
	{
		zombieMap[zombieNumber]->AddPlayerToInRangeMap(playerMap[playerNumber]);
	}
}

void GameServer::ProcessOutRangeZombie(SocketInfo* socketInfo, stringstream& recvStream)
{
	int playerNumber = socketInfo->number, zombieNumber = -1;
	recvStream >> zombieNumber;

	if (zombieMap.find(zombieNumber) != zombieMap.end() && playerMap.find(playerNumber) != playerMap.end())
	{
		zombieMap[zombieNumber]->RemoveInRangePlayer(playerNumber);
	}
}

void GameServer::ProcessZombieHitResult(SocketInfo* socketInfo, std::stringstream& recvStream)
{
	EnterCriticalSection(&critsecPlayerInfo);

	int zombieNumber = -1;
	bool bResult = false;
	HitInfo hitInfo;
	recvStream >> zombieNumber >> bResult >> hitInfo;
	
	zombieMap[zombieNumber]->ChangeState();

	if (bResult && zombieMap.find(zombieNumber) != zombieMap.end())
	{
		stringstream healthChangedStream, hitInfoStream;

		hitInfoStream << static_cast<int>(EPacketType::ZOMBIE_HITS_ME) << "\n";
		hitInfoStream << hitInfo;

		healthChangedStream << static_cast<int>(EPacketType::HEALTH_CHANGED) << "\n";
		healthChangedStream << hitInfo.characterNumber << "\n";

		playerMap[hitInfo.characterNumber]->TakeDamage(zombieMap[zombieNumber]->GetAttackPower());
		healthChangedStream << playerMap[socketInfo->number]->GetHealth() << "\n";

		healthChangedStream << hitInfo.isPlayer << "\n";

		Broadcast(healthChangedStream);
		Broadcast(hitInfoStream, socketInfo->number);
	}
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::UpdatePlayerItemGridPoint(SocketInfo* socketInfo, stringstream& recvStream)
{
	string itemID;
	int xPoint, yPoint;
	bool isRotated;
	recvStream >> itemID >> xPoint >> yPoint >> isRotated;

	// ************************************************
	// * 아이템이 해당 인덱스에 위치할 수 있는지 검사 *
	// ************************************************

	bool isPlayerHasItem = playerMap[socketInfo->number]->IsPlayerHasItemInInventory(itemID);

	if (isPlayerHasItem)
	{
		auto item = itemManager->GetItem(itemID);
		GridPoint pointToAdd = { xPoint,yPoint };
		const bool result = playerMap[socketInfo->number]->UpdateItemGridPoint(item, itemID, pointToAdd, isRotated);

		stringstream sendStream;
		sendStream << static_cast<int>(EPacketType::ITEM_GRID_POINT_UPDATE) << "\n";
		sendStream << itemID << "\n";

		if (result)
		{
			sendStream << xPoint << "\n";
			sendStream << yPoint << "\n";
		}
		else
		{
			const GridPoint addedPoint = playerMap[socketInfo->number]->GetItemsAddedPoint(itemID);
			sendStream << addedPoint.x << "\n";
			sendStream << addedPoint.y << "\n";
		}
		sendStream << item->isRotated << "\n";

		Send(socketInfo, sendStream);
	}
}

void GameServer::ActivateWeaponAbility(SocketInfo* socketInfo, std::stringstream& recvStream)
{
	int inputType = 0;
	recvStream >> inputType;

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::ACTIVATE_WEAPON_ABILITY) << "\n";
	sendStream << socketInfo->number << "\n";
	sendStream << inputType << "\n";

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(sendStream, socketInfo->number);
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::ProcessPlayerAttackResult(SocketInfo* socketInfo, stringstream& recvStream)
{
	HitInfo hitInfo;
	string itemID;
	recvStream >> itemID >> hitInfo;

	stringstream hitInfoStream;

	hitInfoStream << static_cast<int>(EPacketType::ATTACK_RESULT) << "\n";
	hitInfoStream << itemID << "\n";
	hitInfoStream << hitInfo << "\n";

	const float weaponAttackPower = itemManager->GetWeaponAttackPower(itemID);

	if (hitInfo.isPlayer)
	{
		if (playerMap[hitInfo.characterNumber] == nullptr)
		{
			playerMap.erase(hitInfo.characterNumber);
		}
		else
		{
			playerMap[hitInfo.characterNumber]->TakeDamage(weaponAttackPower);
		}
	}
	else
	{
		if (zombieMap[hitInfo.characterNumber] == nullptr)
		{
			zombieMap.erase(hitInfo.characterNumber);
		}
		else
		{
			zombieMap[hitInfo.characterNumber]->TakeDamage(weaponAttackPower);
		}
	}

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(hitInfoStream, socketInfo->number);
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::ProcessKickedCharacters(SocketInfo* socketInfo, stringstream& recvStream)
{
	int number = 0;
	bool isPlayer = false;
	recvStream >> number >> isPlayer;

	stringstream sendStream;

	sendStream << static_cast<int>(EPacketType::KICKED_CHARACTERS) << "\n";
	sendStream << socketInfo->number << "\n";
	sendStream << number << "\n";
	sendStream << isPlayer << "\n";

	EnterCriticalSection(&critsecZombieInfo);
	if (isPlayer)
	{
		if (playerMap.find(number) != playerMap.end())
		{
			shared_ptr<Player> kickedPlayer = playerMap[number];
			if (kickedPlayer->GetWrestleState() == EWrestleState::WRESTLING)
			{
				kickedPlayer->WrestlStateOff();
				const int zombieNumber = kickedPlayer->GetZombieNumberWrestleWith();
				if (zombieMap.find(zombieNumber) != zombieMap.end())
				{
					zombieMap[zombieNumber]->ClearStateStatus();
					zombieMap[zombieNumber]->SetZombieState(WaitState::GetInstance());

					sendStream << true << "\n";
					sendStream << zombieNumber << "\n";
				}
				else
				{
					sendStream << false << "\n";
				}
			}
		}
	}
	else
	{
		if (zombieMap.find(number) != zombieMap.end())
		{
			auto targetPlayer = zombieMap[number]->GetTargetPlayer();
			if (targetPlayer && targetPlayer->GetWrestleState() == EWrestleState::WRESTLING)
			{
				CancelPlayerWrestling(targetPlayer);
				targetPlayer->WrestlStateOff();

				sendStream << true << "\n";
				sendStream << targetPlayer->GetNumber() << "\n";
			}
			else
			{
				sendStream << false << "\n";
			}

			zombieMap[number]->ClearStateStatus();
			zombieMap[number]->SetZombieState(WaitState::GetInstance());
		}
	}
	LeaveCriticalSection(&critsecZombieInfo);

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(sendStream);
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::CancelPlayerWrestling(std::weak_ptr<Player> playerPtr)
{
	EnterCriticalSection(&critsecPlayerInfo);
	if (auto playerSharedPtr = playerPtr.lock())
	{
		stringstream sendStream;
		sendStream << static_cast<int>(EPacketType::WRESTLING_CANCELED) << "\n";
		sendStream << playerSharedPtr->GetNumber() << "\n";
		Broadcast(sendStream);
	}
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::ReplicateProjectile(SocketInfo* socketInfo, std::stringstream& recvStream)
{
	stringstream sendStream;

	sendStream << static_cast<int>(EPacketType::PROJECTILE) << "\n";

	Vector3D location;
	Rotator rotation;

	recvStream >> location.X >> location.Y >> location.Z;
	recvStream >> rotation.pitch >> rotation.yaw >> rotation.roll;
	sendStream << socketInfo->number << "\n";
	sendStream << location << rotation;

	Broadcast(sendStream, socketInfo->number);
}

void GameServer::SynchronizeItemPickingUp(SocketInfo* socketInfo, stringstream& recvStream)
{
	string itemID;
	recvStream >> itemID;

	// ************************************************
	// *플레이어가 해당 아이템을 획득할 수 있는지 검사*
	// ************************************************

	auto item = itemManager->GetItem(itemID);
	const bool result = playerMap[socketInfo->number]->TryAddItem(item, itemID);

	itemManager->SetItemStateToDeactivated(itemID);

	if (result)
	{
		stringstream sendStream;
		sendStream << static_cast<int>(EPacketType::PICKUP_ITEM) << "\n";
		sendStream << socketInfo->number << "\n";
		sendStream << itemID << "\n";

		EnterCriticalSection(&critsecPlayerInfo);
		Broadcast(sendStream, socketInfo->number);
		LeaveCriticalSection(&critsecPlayerInfo);

		const GridPoint addedPoint = playerMap[socketInfo->number]->GetItemsAddedPoint(itemID);

		sendStream << item->isRotated << "\n";
		sendStream << addedPoint.x << "\n";
		sendStream << addedPoint.y << "\n";

		Send(socketInfo, sendStream);
	}
}

void GameServer::SynchronizeItemDropping(SocketInfo* socketInfo, stringstream& recvStream)
{
	string itemID;
	recvStream >> itemID;

	stringstream sendStream;

	EnterCriticalSection(&critsecPlayerInfo);

	bool isPlayerHasItem = playerMap[socketInfo->number]->IsPlayerHasItemInInventory(itemID);

	sendStream << static_cast<int>(EPacketType::DROP_ITEM) << "\n";
	sendStream << socketInfo->number << "\n";
	sendStream << itemID << "\n";
	sendStream << isPlayerHasItem << "\n";

	if (isPlayerHasItem)
	{
		itemManager->SetItemStateToActivated(itemID);

		auto item = itemManager->GetItem(itemID);
		playerMap[socketInfo->number]->RemoveItemInInventory(item, itemID);
		sendStream << item->itemInfo.itemKey << "\n";
		sendStream << item->itemInfo.quantity << "\n";
		Broadcast(sendStream);
	}
	else
	{
		Send(socketInfo, sendStream);
	}

	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::SynchronizeItemEquipping(SocketInfo* socketInfo, stringstream& recvStream)
{
	string itemID;
	int slotNumber;
	recvStream >> itemID >> slotNumber;

	// ************************************************
	// *플레이어가 해당 아이템을 장착할 수 있는지 검사*
	// 장착할 수 있으면 아이템 매니저 및 플레이어에서 관련 처리
	// 결과 bool값 리턴
	// true  : 브로드캐스트
	// false : 해당 플레이어에게만 결과 전송
	// ************************************************

	bool isPlayerHasItem = playerMap[socketInfo->number]->IsPlayerHasItemInInventory(itemID);

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::EQUIP_ITEM) << "\n";
	sendStream << socketInfo->number << "\n";
	sendStream << itemID << "\n";
	sendStream << slotNumber << "\n";

	if (isPlayerHasItem)
	{
		auto item = itemManager->GetItem(itemID);
		playerMap[socketInfo->number]->ItemEquipFromInventory(item, itemID, slotNumber);

		EnterCriticalSection(&critsecPlayerInfo);
		Broadcast(sendStream, socketInfo->number);
		LeaveCriticalSection(&critsecPlayerInfo);
	}

	sendStream << isPlayerHasItem << "\n";
	Send(socketInfo, sendStream);
}

void GameServer::SynchronizeItemUnequipping(SocketInfo* socketInfo, stringstream& recvStream)
{
	string itemID;
	int xPoint, yPoint;
	recvStream >> itemID >> xPoint >> yPoint;

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::UNEQUIP_ITEM) << "\n";
	sendStream << socketInfo->number << "\n";
	sendStream << itemID << "\n";
	const bool isPlayerHasEquippedItem = playerMap[socketInfo->number]->IsPlayerHasItemInEquipment(itemID);

	if (isPlayerHasEquippedItem)
	{
		auto item = itemManager->GetItem(itemID);
		GridPoint pointToAdd{ xPoint ,yPoint };
		const bool result = playerMap[socketInfo->number]->TryAddItemAt(item, itemID, pointToAdd);

		if (result)
		{
			EnterCriticalSection(&critsecPlayerInfo);
			Broadcast(sendStream, socketInfo->number);
			LeaveCriticalSection(&critsecPlayerInfo);

			sendStream << result << "\n";
			sendStream << item->isRotated << "\n";
			sendStream << pointToAdd.x << "\n";
			sendStream << pointToAdd.y << "\n";

			Send(socketInfo, sendStream);
			return;
		}
	}
	
	sendStream << isPlayerHasEquippedItem << "\n";
	Send(socketInfo, sendStream);
}

void GameServer::SynchronizeEquippedItemDropping(SocketInfo* socketInfo, stringstream& recvStream)
{
	string itemID;
	recvStream >> itemID;

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::DROP_EQUIPPED_ITEM) << "\n";
	sendStream << socketInfo->number << "\n";
	sendStream << itemID << "\n";

	bool isPlayerHasEquippedItem = playerMap[socketInfo->number]->IsPlayerHasItemInEquipment(itemID);
	if (isPlayerHasEquippedItem)
	{
		if (itemID == playerMap[socketInfo->number]->GetArmedWeaponID())
		{
			playerMap[socketInfo->number]->DisarmWeapon();
		}
		playerMap[socketInfo->number]->RemoveItemInEquipment(itemID);

		EnterCriticalSection(&critsecPlayerInfo);
		Broadcast(sendStream, socketInfo->number);
		LeaveCriticalSection(&critsecPlayerInfo);
	}
	sendStream << isPlayerHasEquippedItem << "\n";
	Send(socketInfo, sendStream);
}

void GameServer::SynchronizeWeaponArming(SocketInfo* socketInfo, std::stringstream& recvStream)
{
	string itemID;
	recvStream >> itemID;

	bool isPlayerHasEquippedItem = playerMap[socketInfo->number]->IsPlayerHasItemInEquipment(itemID);
	if (isPlayerHasEquippedItem)
	{
		playerMap[socketInfo->number]->ArmWeapon(itemID);

		stringstream sendStream;

		sendStream << static_cast<int>(EPacketType::ARM_WEAPON) << "\n";
		sendStream << socketInfo->number << "\n";
		sendStream << itemID << "\n";

		EnterCriticalSection(&critsecPlayerInfo);
		Broadcast(sendStream, socketInfo->number);
		LeaveCriticalSection(&critsecPlayerInfo);
	}
}

void GameServer::SynchronizeWeaponDisarming(SocketInfo* socketInfo, std::stringstream& recvStream)
{
	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::DISARM_WEAPON) << "\n";
	sendStream << socketInfo->number << "\n";

	playerMap[socketInfo->number]->DisarmWeapon();

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(sendStream, socketInfo->number);
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::SynchronizeWeaponChanging(SocketInfo* socketInfo, std::stringstream& recvStream)
{
	string changedWeaponID;
	recvStream >> changedWeaponID;

	bool isPlayerHasEquippedItem = playerMap[socketInfo->number]->IsPlayerHasItemInEquipment(changedWeaponID);
	if (isPlayerHasEquippedItem)
	{
		playerMap[socketInfo->number]->ArmWeapon(changedWeaponID);

		stringstream sendStream;

		sendStream << static_cast<int>(EPacketType::CHANGE_WEAPON) << "\n";
		sendStream << socketInfo->number << "\n";
		sendStream << changedWeaponID << "\n";

		EnterCriticalSection(&critsecPlayerInfo);
		Broadcast(sendStream, socketInfo->number);
		LeaveCriticalSection(&critsecPlayerInfo);
	}
}

void GameServer::SynchronizeItemUsing(SocketInfo* socketInfo, std::stringstream& recvStream)
{
	string itemID;
	int consumedAmount = 0;
	recvStream >> itemID >> consumedAmount;

	// 아이템 사용 효과 반영
	itemManager->UseItem(playerMap[socketInfo->number], itemID, consumedAmount);

	stringstream itemUsingStream;
	itemUsingStream << static_cast<int>(EPacketType::USING_ITEM) << "\n";
	itemUsingStream << socketInfo->number << "\n";
	itemUsingStream << itemID << "\n";
	itemUsingStream << consumedAmount << "\n";

	Broadcast(itemUsingStream, socketInfo->number);
}

void GameServer::Broadcast(stringstream& sendStream, const int skipNumber)
{
	for (auto& info : playerSocketMap)
	{
		if (skipNumber != -1 && info.first == skipNumber) continue;
		Send(info.second, sendStream);
	}
}


#include "GameServer.h"
#include "JsonComponent.h"
#include "Item/ItemManager.h"
#include "Structs/ItemInfo.h"
#include "Player/Player.h"
#include "Zombie/ZombieManager.h"
#include "Zombie/Zombie.h"
#include "Zombie/State/IdleState.h"
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

	packetCallbacks[EPacketType::SIGNUP]				= SignUp;
	packetCallbacks[EPacketType::LOGIN]					= Login;
	packetCallbacks[EPacketType::SPAWNPLAYER]			= NewPlayerAccessToGameMap;
	packetCallbacks[EPacketType::SYNCHPLAYER]			= SynchronizePlayerInfo;
	packetCallbacks[EPacketType::PLAYERINPUTACTION]		= BroadcastPlayerInputAction;
	packetCallbacks[EPacketType::ZOMBIEINRANGE]			= ProcessInRangeZombie;
	packetCallbacks[EPacketType::ZOMBIEOUTRANGE]		= ProcessOutRangeZombie;
	packetCallbacks[EPacketType::WRESTLINGRESULT]		= ProcessPlayerWrestlingResult;
	packetCallbacks[EPacketType::PICKUP_ITEM]			= PlayerItemPickUp;
	packetCallbacks[EPacketType::ITEMGRIDPOINTUPDATE]	= PlayerItemGridPointUpdate;
	packetCallbacks[EPacketType::EQUIP_ITEM]			= PlayerItemEquip;
	packetCallbacks[EPacketType::DROP_ITEM]				= PlayerItemDrop;
	packetCallbacks[EPacketType::DROP_EQUIPPED_ITEM]	= PlayerDropEquippedItem;
	packetCallbacks[EPacketType::UNEQUIP_ITEM]			= PlayerUnequipItem;
	packetCallbacks[EPacketType::ATTACKRESULT]			= ProcessPlayerAttackResult;
	packetCallbacks[EPacketType::PLAYERRESPAWN]			= RespawnPlayer;
	packetCallbacks[EPacketType::ZOMBIEHITSME]			= ProcessZombieHitResult;
	packetCallbacks[EPacketType::PROJECTILE]			= ReplicateProjectile;
	packetCallbacks[EPacketType::USINGITEM]				= PlayerUseItem;
	packetCallbacks[EPacketType::CHANGE_WEAPON]			= PlayerChangedWeapon;
	packetCallbacks[EPacketType::ARM_WEAPON]			= PlayerArmWeapon;
	packetCallbacks[EPacketType::DISARM_WEAPON]			= PlayerDisarmWeapon;

	InitializeCriticalSection(&critsecPlayerInfo);

	return true;
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
				Vector3D location{ 800,-1000,97 };
				Rotator rotation{ 0,120,0 };
				shared_ptr<Zombie> zombie = zombieManager->GetZombie(location, rotation);
				zombie->AllZombieInfoBitOn();
				zombieMap[zombie->GetNumber()] = zombie;
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
				sendStream << static_cast<int>(EPacketType::SYNCHZOMBIE) << "\n";
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

	Vector3D location{ 800,-1000,97 };
	Rotator rotation{ 0,120,0 };

	shared_ptr<Zombie> zombie = zombieManager->GetZombie(location, rotation);
	zombie->RegisterZombieDeadCallback(ProcessZombieDead);
	zombieMap[zombie->GetNumber()] = zombie;

	location.X = 1100;
	location.Y = 900;
	rotation.yaw = -120;

	shared_ptr<Zombie> zombie2 = zombieManager->GetZombie(location, rotation);
	zombie2->RegisterZombieDeadCallback(ProcessZombieDead);
	zombieMap[zombie2->GetNumber()] = zombie2;
}

void GameServer::HandleDisconnectedClient(SocketInfo* socketInfo)
{
	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::PLAYERDISCONNECTED) << "\n";
	sendStream << socketInfo->number << "\n";

	cout << "[Log] : " << socketInfo->number << "�� Ŭ���̾�Ʈ (ID : " << playerIDMap[socketInfo->number]  << ") ���� ����\n";

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
	sendStream << dbConnector->PlayerSignUp(id, pw) << "\n";
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

	cout << "[Log] : ID " << id << " �α��� ��û -> ���(" << isLoginSuccess << ")\n";
}

void GameServer::NewPlayerAccessToGameMap(SocketInfo* socketInfo, stringstream& recvStream)
{
	cout << "[Log] : " << socketInfo->number << "�� Ŭ���̾�Ʈ ���� �ʿ� ����\n";

	// ������ �÷��̾��� �κ��丮 ������ DB���� �ҷ�����
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

	// �÷��̾� ��ü ����
	playerMap[socketInfo->number] = make_shared<Player>(socketInfo->number, playerInfo);

	auto player = playerMap[socketInfo->number];
	player->PlayerInGameMap();

	// Ŭ���̾�Ʈ�� ���� ��Ŷ ����
	stringstream initialInfoStream;
	initialInfoStream << static_cast<int>(EPacketType::WORLDINITIALINFO) << "\n";

	initialInfoStream << static_cast<int>(EPacketType::PLAYERINITIALINFO) << "\n";
	player->SerializePlayerInitialInfo(initialInfoStream);

	// ���� ������ ����ȭ
	//SaveZombieInfoToPacket(initialInfoStream);	

	// �ʵ忡 �ִ� ������ ������ ����ȭ
	SaveItemInfoToPacket(initialInfoStream);

	// �κ��丮 ������ ����ȭ
	if (inventoryResult)
	{
		initialInfoStream << static_cast<int>(EPacketType::PLAYERINVENTORY) << "\n";
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
		initialInfoStream << static_cast<int>(EPacketType::PLAYEREQUIPMENT) << "\n";
		initialInfoStream << equippedItems.size() << "\n";
		for (auto& equipped : equippedItems)
		{
			player->ItemEquipInitialize(equipped.itemID, equipped.slotNumber);
			initialInfoStream << equipped;
		}
	}

	// �ٸ� �÷��̾���� ������ ����ȭ
	SerializeOthersToNewPlayer(socketInfo->number, initialInfoStream);

	Send(socketInfo, initialInfoStream);

	// ������ �÷��̾�鿡�� ��� ������ �÷��̾� ������ ����
	SerializeNewPlayerToOthers(player, socketInfo->number, recvStream);

	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::SerializeOthersToNewPlayer(const int playerNumber, std::stringstream& sendStream)
{
	stringstream otherPlayersStream;
	int count = 0;
	for (auto& p : playerMap)
	{
		if (p.first == playerNumber) 
			continue;

		otherPlayersStream << p.first << "\n";					// �÷��̾� ��ȣ
		otherPlayersStream << playerIDMap[p.first] << "\n";		// �÷��̾� ���̵�
		p.second->SerializeData(otherPlayersStream);			// �÷��̾� ������
		
		// �÷��̾� �������ͽ� : ü��
		auto status = p.second->GetPlayerStatus();
		otherPlayersStream << status;

		// ���� ���� ������
		SerializePlayersEquippedItems(p.second, otherPlayersStream);

		count++;
	}
	if (count)
	{
		sendStream << static_cast<int>(EPacketType::SPAWNPLAYER) << "\n";
		sendStream << count << "\n"; // �÷��̾� ��
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
	// �ݹ� �Լ� ���
	player->RegisterWrestlingCallback(ProcessPlayerWrestlingStart);
	player->RegisterPlayerDeadCallback(ProcessPlayerDead);

	// ��� ������ �÷��̾��� �����͸� ������ȭ
	player->DeserializeData(recvStream);

	// ��� ������ �÷��̾��� ������ ����ȭ
	stringstream newPlayerInfoStream;
	newPlayerInfoStream << static_cast<int>(EPacketType::SPAWNPLAYER) << "\n";
	newPlayerInfoStream << 1 << "\n";
	newPlayerInfoStream << playerNumber << "\n";
	newPlayerInfoStream << playerIDMap[playerNumber] << "\n";
	player->SerializeData(newPlayerInfoStream);

	// �÷��̾� �������ͽ� : ü��
	auto status = player->GetPlayerStatus();
	newPlayerInfoStream << status;

	// ���� ���� ������
	SerializePlayersEquippedItems(player, newPlayerInfoStream);

	Broadcast(newPlayerInfoStream, playerNumber);
}

void GameServer::SaveZombieInfoToPacket(stringstream& sendStream)
{
	sendStream << static_cast<int>(EPacketType::SYNCHZOMBIE) << "\n";
	sendStream << zombieMap.size() << "\n";
	for (auto& kv : zombieMap)
	{
		kv.second->AllZombieInfoBitOn();
		kv.second->SerializeData(sendStream);
	}
}

void GameServer::SaveItemInfoToPacket(std::stringstream& sendStream)
{
	sendStream << static_cast<int>(EPacketType::SPAWNITEM) << "\n";
	itemManager->SaveItemInfoToPacket(sendStream);
}

void GameServer::SynchronizePlayerInfo(SocketInfo* socketInfo, stringstream& recvStream)
{
	EnterCriticalSection(&critsecPlayerInfo);

	if (playerMap.find(socketInfo->number) == playerMap.end())
	{
		LeaveCriticalSection(&critsecPlayerInfo);
		return;
	}

	playerMap[socketInfo->number]->DeserializeData(recvStream);
	playerMap[socketInfo->number]->Waiting();
	
	int count = 0;
	stringstream sendStream, dataStream;
	for (auto& kv : playerMap)
	{
		if (kv.second->GetIsDead())
			continue;
		dataStream << kv.first << "\n";
		kv.second->SerializeData(dataStream);
		count++;
	}
	sendStream << static_cast<int>(EPacketType::SYNCHPLAYER) << "\n";
	sendStream << count << "\n";
	sendStream << dataStream.str();

	LeaveCriticalSection(&critsecPlayerInfo);

	Send(socketInfo, sendStream);
}

void GameServer::BroadcastPlayerInputAction(SocketInfo* socketInfo, stringstream& recvStream)
{
	int inputType = 0, weaponType = 0;
	recvStream >> inputType >> weaponType;

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::PLAYERINPUTACTION) << "\n";
	sendStream << socketInfo->number << "\n";
	sendStream << inputType << "\n";
	sendStream << weaponType << "\n";

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(sendStream, socketInfo->number);
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

void GameServer::ProcessPlayerWrestlingResult(SocketInfo* socketInfo, stringstream& recvStream)
{
	bool wrestlingResult;
	recvStream >> wrestlingResult;

	shared_ptr<Player> player = playerMap[socketInfo->number];
	player->WrestlStateOff();
	player->SetSuccessToBlocking(wrestlingResult);

	zombieMap[player->GetZombieNumberWrestleWith()]->ChangeState();

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::WRESTLINGRESULT) << "\n";
	sendStream << socketInfo->number << "\n";
	sendStream << wrestlingResult << "\n";

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(sendStream);
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::ProcessPlayerWrestlingStart(const int playerNumber)
{
	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::WRESTLINGSTART) << "\n";
	sendStream << playerNumber << "\n";

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(sendStream);
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::ProcessZombieHitResult(SocketInfo* socketInfo, std::stringstream& recvStream)
{
	EnterCriticalSection(&critsecPlayerInfo);

	int zombieNumber = -1;
	bool bResult = false;
	recvStream >> zombieNumber >> bResult;

	if (zombieMap.find(zombieNumber) != zombieMap.end())
	{
		zombieMap[zombieNumber]->ChangeState();
		cout << "���� " << zombieNumber << "�� �÷��̾� " << socketInfo->number << " �� ���Ƚ��ϴ�.\n";
	}
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::PlayerItemPickUp(SocketInfo* socketInfo, stringstream& recvStream)
{
	string itemID;
	recvStream >> itemID;

	// ************************************************
	// *�÷��̾ �ش� �������� ȹ���� �� �ִ��� �˻�*
	// ************************************************

	auto item = itemManager->GetItem(itemID);
	const bool result = playerMap[socketInfo->number]->TryAddItem(item, itemID);

	//itemManager->SetItemStateToDeactivated(itemID);

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

void GameServer::PlayerItemGridPointUpdate(SocketInfo* socketInfo, stringstream& recvStream)
{
	string itemID;
	int xPoint, yPoint;
	bool isRotated;
	recvStream >> itemID >> xPoint >> yPoint >> isRotated;
	
	// ************************************************
	// * �������� �ش� �ε����� ��ġ�� �� �ִ��� �˻� *
	// ************************************************

	bool isPlayerHasItem = playerMap[socketInfo->number]->IsPlayerHasItemInInventory(itemID);

	if (isPlayerHasItem)
	{
		auto item = itemManager->GetItem(itemID);
		GridPoint pointToAdd = { xPoint,yPoint };
		const bool result = playerMap[socketInfo->number]->UpdateItemGridPoint(item, itemID, pointToAdd, isRotated);

		stringstream sendStream;
		sendStream << static_cast<int>(EPacketType::ITEMGRIDPOINTUPDATE) << "\n";
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

void GameServer::PlayerItemEquip(SocketInfo* socketInfo, stringstream& recvStream)
{
	string itemID;
	int slotNumber;
	recvStream >> itemID >> slotNumber;

	// ************************************************
	// *�÷��̾ �ش� �������� ������ �� �ִ��� �˻�*
	// ������ �� ������ ������ �Ŵ��� �� �÷��̾�� ���� ó��
	// ��� bool�� ����
	// true  : ��ε�ĳ��Ʈ
	// false : �ش� �÷��̾�Ը� ��� ����
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

void GameServer::PlayerUnequipItem(SocketInfo* socketInfo, stringstream& recvStream)
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

		//itemManager->SetItemStateToDeactivated(itemID);
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

void GameServer::PlayerItemDrop(SocketInfo* socketInfo, stringstream& recvStream)
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

void GameServer::Broadcast(stringstream& sendStream, const int skipNumber)
{
	for (auto& info : playerSocketMap)
	{
		if (skipNumber != -1 && info.first == skipNumber) continue;
		Send(info.second, sendStream);
	}
}

void GameServer::ProcessPlayerAttackResult(SocketInfo* socketInfo, stringstream& recvStream)
{
	int size = 0, characterNumber = -1;
	bool isPlayer = false;
	recvStream >> size;
	for (int i = 0; i < size; i++)
	{
		recvStream >> characterNumber >> isPlayer;
		if (isPlayer)
		{
			cout << "Ŭ���̾�Ʈ " << socketInfo->number << "�� Ŭ���̾�Ʈ " << characterNumber << "�� ���Ƚ��ϴ�.\n";
			playerMap[characterNumber]->TakeDamage(100);

			// ��ȭ�� ���� ����
		}
		else
		{
			cout << "�÷��̾� " << socketInfo->number << "�� ���� " << characterNumber << "�� ���Ƚ��ϴ�." << "\n";
			zombieMap[characterNumber]->TakeDamage(100);

			// ��ȭ�� ���� ����
		}
	}

	EnterCriticalSection(&critsecPlayerInfo);
	//Broadcast(sendStream);
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::ProcessPlayerDead(const int playerNumber)
{
	cout << "�÷��̾� " << playerNumber << "�� �׾����ϴ�.\n";

	EnterCriticalSection(&critsecPlayerInfo);

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::PLAYERDEAD) << "\n";
	sendStream << playerNumber << "\n";
	Broadcast(sendStream);
	
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::PlayerDropEquippedItem(SocketInfo* socketInfo, stringstream& recvStream)
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

void GameServer::ProcessZombieDead(const int zombieNumber)
{
	cout << "���� " << zombieNumber << "�� �׾����ϴ�.\n";

	EnterCriticalSection(&critsecZombieInfo);
	shared_ptr<Zombie> zombie = zombieMap[zombieNumber];
	zombieMap.erase(zombieNumber);
	LeaveCriticalSection(&critsecZombieInfo);

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::ZOMBIEDEAD) << "\n";
	sendStream << zombieNumber << "\n";
	Broadcast(sendStream);
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

		// ���� ���� ���� ���·� �׾��ٸ� �������� ���·� ������
		stringstream sendStream;
		sendStream << static_cast<int>(EPacketType::PLAYERRESPAWN) << "\n";
		sendStream << socketInfo->number << "\n";
		playerMap[socketInfo->number]->SerializeData(sendStream);
		Broadcast(sendStream);
	}
}

void GameServer::ReplicateProjectile(SocketInfo* socketInfo, std::stringstream& recvStream)
{
	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::PROJECTILE) << "\n";
	sendStream << recvStream.str() << "\n";

	Broadcast(sendStream, socketInfo->number);
}

void GameServer::PlayerUseItem(SocketInfo* socketInfo, std::stringstream& recvStream)
{
	string itemID;
	int consumedAmount = 0;
	recvStream >> itemID >> consumedAmount;

	// ������ ��� ȿ�� �ݿ�
	itemManager->UseItem(playerMap[socketInfo->number], itemID, consumedAmount);

	stringstream itemUsingStream;
	itemUsingStream << static_cast<int>(EPacketType::USINGITEM) << "\n";
	itemUsingStream << socketInfo->number << "\n";
	itemUsingStream << itemID << "\n";
	itemUsingStream << consumedAmount << "\n";

	Broadcast(itemUsingStream, socketInfo->number);

	stringstream playerStatusStream;
	auto status = playerMap[socketInfo->number]->GetPlayerStatus();
	playerStatusStream << static_cast<int>(EPacketType::PLAYERSTATUS) << "\n";
	playerStatusStream << socketInfo->number << "\n";
	playerStatusStream << status;

	Broadcast(playerStatusStream);
}

void GameServer::DestroyItem(const int playerNumber, shared_ptr<Item> item, const string& itemID)
{
	// ������ �ı�
	playerMap[playerNumber]->RemoveItemInInventory(item, itemID);
}

void GameServer::PlayerChangedWeapon(SocketInfo* socketInfo, std::stringstream& recvStream)
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

void GameServer::PlayerArmWeapon(SocketInfo* socketInfo, std::stringstream& recvStream)
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

void GameServer::PlayerDisarmWeapon(SocketInfo* socketInfo, std::stringstream& recvStream)
{
	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::DISARM_WEAPON) << "\n";
	sendStream << socketInfo->number << "\n";

	playerMap[socketInfo->number]->DisarmWeapon();

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(sendStream, socketInfo->number);
	LeaveCriticalSection(&critsecPlayerInfo);
}
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

	itemManager = make_unique<ItemManager>();

	packetCallbacks[EPacketType::SIGNUP]				= SignUp;
	packetCallbacks[EPacketType::LOGIN]					= Login;
	packetCallbacks[EPacketType::SPAWNPLAYER]			= SpawnOtherPlayers;
	packetCallbacks[EPacketType::SYNCHPLAYER]			= SynchronizePlayerInfo;
	packetCallbacks[EPacketType::PLAYERINPUTACTION]		= BroadcastPlayerInputAction;
	packetCallbacks[EPacketType::ZOMBIEINRANGE]			= ProcessInRangeZombie;
	packetCallbacks[EPacketType::ZOMBIEOUTRANGE]		= ProcessOutRangeZombie;
	packetCallbacks[EPacketType::WRESTLINGRESULT]		= ProcessPlayerWrestlingResult;
	packetCallbacks[EPacketType::ITEMTOPICKUP]			= PlayerItemPickUp;
	packetCallbacks[EPacketType::ITEMGRIDPOINTUPDATE]	= PlayerItemGridPointUpdate;
	packetCallbacks[EPacketType::ITEMTOEQUIP]			= PlayerItemEquip;
	packetCallbacks[EPacketType::ITEMTODROP]			= PlayerItemDrop;
	packetCallbacks[EPacketType::ATTACKRESULT]			= ProcessPlayerAttackResult;
	packetCallbacks[EPacketType::PLAYERRESPAWN]			= RespawnPlayer;
	packetCallbacks[EPacketType::ZOMBIEHITSME]			= ProcessZombieHitResult;

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

	// �÷��̾ ���� �����۵� ó��
	SavePlayerInfo(socketInfo->number);
	RemovePlayerInfo(socketInfo->number);
	
	// ���� ī��Ʈ Ȯ��

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(sendStream, socketInfo->number);
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::SavePlayerInfo(const int playerNumber)
{
	auto& playerInventoryStatus = playerMap[playerNumber]->GetInventoryStatus();
	vector<PlayerItems> playerItems;
	for (auto& itemStatus : playerInventoryStatus)
	{
		PlayerItems playerItem;
		auto item = itemManager->GetItem(itemStatus.first);
		playerItem.itemID = itemStatus.first;
		playerItem.quantity = item->itemInfo.count;
		playerItem.topLeftX = item->gridPoint.x;
		playerItem.topLeftY = item->gridPoint.y;
		playerItem.isRotated = item->isRotated;
		playerItem.isEquipped = itemStatus.second.isEquipped;
		playerItem.equippedSlotNumber = itemStatus.second.slotNumber;

		playerItems.push_back(playerItem);
	}
	dbConnector->SavePlayerStatus(playerIDMap[playerNumber], playerItems);
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
		sendStream << socketInfo->number << "\n";
	}
	Send(socketInfo, sendStream);

	cout << "[Log] : ID " << id << " �α��� ��û -> ���(" << isLoginSuccess << ")\n";
}

void GameServer::SpawnOtherPlayers(SocketInfo* socketInfo, stringstream& recvStream)
{
	cout << "[Log] : " << socketInfo->number << "�� Ŭ���̾�Ʈ ���� �ʿ� ����\n";

	vector<PlayerItems> playerItems;
	dbConnector->GetPlayersItems("hyk3662", playerItems);

	EnterCriticalSection(&critsecPlayerInfo);

	// ���� �÷��̾���� ������ ��� ������ �÷��̾�� ���� ��Ʈ���� ����
	stringstream initialInfoStream;
	initialInfoStream << static_cast<int>(EPacketType::INITIALINFO) << "\n";

	//SaveZombieInfoToPacket(initialInfoStream);	// ���� ���� ����ȭ

	SaveItemInfoToPacket(initialInfoStream);	// ������ ���� ����ȭ

	initialInfoStream << static_cast<int>(EPacketType::PLAYERINFO) << "\n";
	initialInfoStream << playerItems.size() << "\n";

	playerMap[socketInfo->number] = make_shared<Player>(socketInfo->number);
	playerMap[socketInfo->number]->PlayerInGameMap();
	auto player = playerMap[socketInfo->number];

	for (PlayerItems& playerItem : playerItems)
	{
		auto item = itemManager->GetItem(playerItem.itemID);
		if (playerItem.isRotated) item->Rotate();
		player->AddItem(item, { playerItem.topLeftX, playerItem.topLeftY }, item->itemInfo.itemGridSize, playerItem.itemID);

		initialInfoStream << playerItem;
	}

	initialInfoStream << static_cast<int>(EPacketType::SPAWNPLAYER) << "\n";
	initialInfoStream << playerMap.size() << "\n";				// �÷��̾� ��
	for (auto& p : playerMap)
	{
		initialInfoStream << playerIDMap[p.first] << "\n";		// �÷��̾� ���̵�
		p.second->SerializeData(initialInfoStream);				// �÷��̾� ����
	}
	Send(socketInfo, initialInfoStream);

	player->RegisterWrestlingCallback(ProcessPlayerWrestlingStart);
	player->RegisterPlayerDeadCallback(ProcessPlayerDead);
	// ��� ������ �÷��̾��� ������ ������ȭ
	player->DeserializeData(recvStream);

	// ��� ������ �÷��̾��� ������ ���� �÷��̾�鿡�� ���� ��Ʈ���� ���� 
	stringstream newPlayerInfoStream;
	newPlayerInfoStream << static_cast<int>(EPacketType::SPAWNPLAYER) << "\n";
	newPlayerInfoStream << 1 << "\n";
	newPlayerInfoStream << playerIDMap[socketInfo->number] << "\n";
	player->SerializeData(newPlayerInfoStream);

	// ������ �÷��̾�鿡�� ��� ������ �÷��̾� ���� ����
	Broadcast(newPlayerInfoStream, socketInfo->number);

	LeaveCriticalSection(&critsecPlayerInfo);
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
	int inputType = 0;
	recvStream >> inputType;

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::PLAYERINPUTACTION) << "\n";
	sendStream << socketInfo->number << "\n";
	sendStream << inputType << "\n";

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
	int itemID;
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
		sendStream << static_cast<int>(EPacketType::ITEMTOPICKUP) << "\n";
		sendStream << socketInfo->number << "\n";
		sendStream << itemID << "\n";

		EnterCriticalSection(&critsecPlayerInfo);
		Broadcast(sendStream, socketInfo->number);
		LeaveCriticalSection(&critsecPlayerInfo);

		sendStream << item->isRotated << "\n";
		sendStream << item->gridPoint.x << "\n";
		sendStream << item->gridPoint.y << "\n";

		Send(socketInfo, sendStream);
		
		//PlayerItems playerItems{ itemID, item->itemInfo.count, item->gridPoint.x, item->gridPoint.y, item->isRotated, 0, -1 };
		//dbConnector->PlayerPickedUpItem(playerIDMap[socketInfo->number], playerItems);
	}
}

void GameServer::PlayerItemGridPointUpdate(SocketInfo* socketInfo, stringstream& recvStream)
{
	int itemID, xPoint, yPoint;
	bool isRotated;
	recvStream >> itemID >> xPoint >> yPoint >> isRotated;
	
	// ************************************************
	// * �������� �ش� �ε����� ��ġ�� �� �ִ��� �˻� *
	// ************************************************

	bool isPlayerHasItem = playerMap[socketInfo->number]->IsPlayerHasItem(itemID);

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
			sendStream << item->gridPoint.x << "\n";
			sendStream << item->gridPoint.y << "\n";
		}
		sendStream << item->isRotated << "\n";

		Send(socketInfo, sendStream);
	}
}

void GameServer::PlayerItemEquip(SocketInfo* socketInfo, stringstream& recvStream)
{
	int itemID, slotNumber;
	recvStream >> itemID >> slotNumber;

	// ************************************************
	// *�÷��̾ �ش� �������� ������ �� �ִ��� �˻�*
	// ������ �� ������ ������ �Ŵ��� �� �÷��̾�� ���� ó��
	// ��� bool�� ����
	// true  : ��ε�ĳ��Ʈ
	// false : �ش� �÷��̾�Ը� ��� ����
	// ************************************************

	bool result = playerMap[socketInfo->number]->IsPlayerHasItem(itemID);

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::ITEMTOEQUIP) << "\n";
	sendStream << socketInfo->number << "\n";
	sendStream << itemID << "\n";

	if (result)
	{
		auto item = itemManager->GetItem(itemID);
		playerMap[socketInfo->number]->PlayerEquipItem(item, itemID, slotNumber);

		EnterCriticalSection(&critsecPlayerInfo);
		Broadcast(sendStream, socketInfo->number);
		LeaveCriticalSection(&critsecPlayerInfo);
	}

	sendStream << slotNumber << "\n";
	sendStream << result << "\n";
	Send(socketInfo, sendStream);
}

void GameServer::PlayerItemDrop(SocketInfo* socketInfo, stringstream& recvStream)
{
	int itemID;
	recvStream >> itemID;

	bool result = playerMap[socketInfo->number]->IsPlayerHasItem(itemID);

	if (result)
	{
		auto item = itemManager->GetItem(itemID);
		playerMap[socketInfo->number]->RemoveItemInInventory(item, itemID);
		playerMap[socketInfo->number]->RemoveItem(itemID);
	}

	// itemManager ������ Ȱ��ȭ

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::ITEMTODROP) << "\n";
	sendStream << socketInfo->number << "\n";
	sendStream << itemID << "\n";

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(sendStream);
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

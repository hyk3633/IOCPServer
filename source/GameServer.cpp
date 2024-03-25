#include "GameServer.h"
#include "Item/ItemManager.h"
#include "Player/Player.h"
#include "Zombie/ZombieManager.h"
#include "Zombie/Zombie.h"
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

	packetCallbacks = vector<void(*)(SocketInfo*, stringstream&)>(static_cast<int>(EPacketType::PACKETTYPE_MAX));
	packetCallbacks[static_cast<int>(EPacketType::SIGNUP)]				= SignUp;
	packetCallbacks[static_cast<int>(EPacketType::LOGIN)]				= Login;
	packetCallbacks[static_cast<int>(EPacketType::SPAWNPLAYER)]			= SpawnOtherPlayers;
	packetCallbacks[static_cast<int>(EPacketType::SYNCHPLAYER)]			= SynchronizePlayerInfo;
	packetCallbacks[static_cast<int>(EPacketType::PLAYERINPUTACTION)]	= BroadcastPlayerInputAction;
	packetCallbacks[static_cast<int>(EPacketType::WRESTLINGRESULT)]		= ProcessPlayerWrestlingResult;
	packetCallbacks[static_cast<int>(EPacketType::SYNCHITEM)]			= SynchronizeItemInfo;
	packetCallbacks[static_cast<int>(EPacketType::HITPLAYER)]			= HitPlayer;
	packetCallbacks[static_cast<int>(EPacketType::HITZOMBIE)]			= HitZombie;

	InitializeCriticalSection(&critsecPlayerInfo);

	return true;
}

void GameServer::ZombieThread()
{
	InitializeZombieInfo();

	while (1)
	{
		if (zombieThreadElapsedTime >= zombiePacketSendingInterval)
		{
			packetFlag = true;
		}
		if (playerSocketMap.size())
		{
			if (zombieMap.size() == 0)
			{
				Vector3D location{ 800,-1000,97 };
				Rotator rotation{ 0,120,0 };
				shared_ptr<Zombie> zombie = zombieManager->GetZombie(location, rotation);
				zombieMap[zombie->GetNumber()] = zombie;
			}

			EnterCriticalSection(&critsecZombieInfo);
			bool sendFlag = false;
			stringstream sendStream;
			sendStream << static_cast<int>(EPacketType::SYNCHZOMBIE) << "\n";
			sendStream << zombieMap.size() << "\n";
			for (auto& kv : zombieMap)
			{
				kv.second->Update();
				if (packetFlag && kv.second->GetSendInfoBit())
				{
					kv.second->SerializeData(sendStream);
					sendFlag = true;
				}
			}
			LeaveCriticalSection(&critsecZombieInfo);

			if (packetFlag && sendFlag)
			{
				EnterCriticalSection(&critsecPlayerInfo);
				Broadcast(sendStream);
				LeaveCriticalSection(&critsecPlayerInfo);
				packetFlag = false;
				zombieThreadElapsedTime = 0;
			}
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

	zombie = zombieManager->GetZombie(location, rotation);
	zombie->RegisterZombieDeadCallback(ProcessZombieDead);
	zombieMap[zombie->GetNumber()] = zombie;

	//zombieMap[1].SetNumber(1);
	//zombieMap[1].SetLocation(location);
	//zombieMap[1].SetRotation(rotation);
}

void GameServer::HandleDisconnectedClient(SocketInfo* socketInfo)
{
	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::PLAYERDISCONNECTED) << "\n";
	sendStream << socketInfo->number << "\n";

	cout << "[Log] : " << socketInfo->number << "번 클라이언트 (ID : " << playerIDMap[socketInfo->number]  << ") 접속 종료\n";

	playerSocketMap.erase(socketInfo->number);					
	playerIDMap.erase(socketInfo->number);		
	playerMap.erase(socketInfo->number);
	// 참조 카운트 확인

	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(sendStream, socketInfo->number);
	LeaveCriticalSection(&critsecPlayerInfo);
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

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::LOGIN) << "\n";
	const bool isLoginSuccess = dbConnector->PlayerLogin(id, pw);
	sendStream << isLoginSuccess << "\n";
	sendStream << playerCount << "\n";
	Send(socketInfo, sendStream);

	cout << "[Log] : ID " << id << " 로그인 요청 -> 결과(" << isLoginSuccess << ")\n";

	EnterCriticalSection(&critsecPlayerInfo);
	playerIDMap[playerCount] = id;
	playerSocketMap[playerCount] = socketInfo;
	socketInfo->number = playerCount;
	playerCount++;
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::SpawnOtherPlayers(SocketInfo* socketInfo, stringstream& recvStream)
{
	cout << "[Log] : " << socketInfo->number << "번 클라이언트 게임 맵에 접속\n";

	EnterCriticalSection(&critsecPlayerInfo);

	// 기존 플레이어들의 정보를 방금 접속한 플레이어에게 보낼 스트림에 저장
	stringstream otherPlayersInfoStream;
	otherPlayersInfoStream << static_cast<int>(EPacketType::SPAWNPLAYER) << "\n";
	otherPlayersInfoStream << playerMap.size() << "\n";				// 플레이어 수
	for (auto& p : playerMap)
	{
		otherPlayersInfoStream << playerIDMap[p.first] << "\n";		// 플레이어 아이디
		p.second->SerializeData(otherPlayersInfoStream);			// 플레이어 정보
	}

	// 방금 접속한 플레이어에게 기존의 플레이어들 정보 전송
	Send(socketInfo, otherPlayersInfoStream);

	// 방금 접속한 플레이어에게 좀비 정보 동기화
	stringstream zombieInfoStream;
	SaveZombieInfoToPacket(zombieInfoStream);
	Send(socketInfo, zombieInfoStream);

	// 방금 접속한 플레이어에게 아이템 정보 동기화
	stringstream itemInfoStream;
	SaveItemInfoToPacket(itemInfoStream);
	Send(socketInfo, itemInfoStream);

	playerMap[socketInfo->number] = make_shared<Player>(socketInfo->number);
	shared_ptr<Player> playerPtr = playerMap[socketInfo->number];
	playerPtr->RegisterWrestlingCallback(ProcessPlayerWrestlingStart);
	playerPtr->RegisterPlayerDeadCallback(ProcessPlayerDead);
	// 방금 접속한 플레이어의 정보를 역직렬화
	playerPtr->DeserializeData(recvStream);

	// 방금 접속한 플레이어의 정보를 기존 플레이어들에게 보낼 스트림에 저장 
	stringstream newPlayerInfoStream;
	newPlayerInfoStream << static_cast<int>(EPacketType::SPAWNPLAYER) << "\n";
	newPlayerInfoStream << 1 << "\n";
	newPlayerInfoStream << playerIDMap[socketInfo->number] << "\n";
	playerPtr->SerializeData(newPlayerInfoStream);

	// 기존의 플레이어들에게 방금 접속한 플레이어 정보 전송
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
	sendStream << static_cast<int>(EPacketType::SYNCHITEM) << "\n";
	itemManager->SaveItemInfoToPacket(sendStream);
}

void GameServer::SynchronizePlayerInfo(SocketInfo* socketInfo, stringstream& recvStream)
{
	EnterCriticalSection(&critsecPlayerInfo);
	playerMap[socketInfo->number]->DeserializeData(recvStream);
	playerMap[socketInfo->number]->DeserializeExtraData(recvStream);
	LeaveCriticalSection(&critsecPlayerInfo);

	ProcessPlayerInfo(playerMap[socketInfo->number]);
	playerMap[socketInfo->number]->Waiting();

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::SYNCHPLAYER) << "\n";
	sendStream << playerMap.size() << "\n";
	for (auto& kv : playerMap)
	{
		kv.second->SerializeData(sendStream);
		kv.second->SerializeExtraData(sendStream);
	}
	Send(socketInfo, sendStream);
}

void GameServer::ProcessPlayerInfo(shared_ptr<Player> player)
{
	const int recvInfoBitMask = player->GetRecvInfoBitMask();
	const int bitMax = static_cast<int>(PIBTC::MAX);
	for (int bit = 0; bit < bitMax; bit++)
	{
		if (recvInfoBitMask & (1 << bit))
		{
			CheckInfoBitAndProcess(player, static_cast<PIBTC>(bit));
		}
	}
}

void GameServer::CheckInfoBitAndProcess(shared_ptr<Player> player, const PIBTC bitType)
{
	switch (bitType)
	{
		case PIBTC::ZombiesInRange:
		{
			for (int number : player->GetZombiesInRange())
			{
				if (zombieMap.find(number) != zombieMap.end())
				{
					zombieMap[number]->AddPlayerToInRangeMap(player);
				}
			}
			break;
		}
		case PIBTC::ZombiesOutRange:
		{
			for (int number : player->GetZombiesOutRange())
			{
				if (zombieMap.find(number) != zombieMap.end()) 
				{
					zombieMap[number]->RemoveInRangePlayer(player->GetNumber());
				}
			}
			break;
		}
		case PIBTC::ZombieAttackResult:
		{
			if (zombieMap.find(player->GetZombieNumberAttackedBy()) != zombieMap.end())
			{
				zombieMap[player->GetZombieNumberAttackedBy()]->ChangeState();
			}
			break;
		}
	}
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
	Broadcast(sendStream, socketInfo->number);
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

void GameServer::SynchronizeItemInfo(SocketInfo* socketInfo, stringstream& recvStream)
{
	int itemNumber;
	recvStream >> itemNumber;

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::DESTROYITEM) << "\n";
	sendStream << itemNumber << "\n";

	itemManager->SetItemStateToDeactivated(itemNumber);
	
	EnterCriticalSection(&critsecPlayerInfo);
	Broadcast(sendStream, socketInfo->number);
	LeaveCriticalSection(&critsecPlayerInfo);

	sendStream.str("");
	sendStream << static_cast<int>(EPacketType::PICKUPITEM) << "\n";
	sendStream << itemNumber << "\n";
	Send(socketInfo, sendStream);
}

void GameServer::Broadcast(stringstream& sendStream, const int skipNumber)
{
	for (auto& info : playerSocketMap)
	{
		if (skipNumber != -1 && info.first == skipNumber) continue;
		Send(info.second, sendStream);
	}
}

void GameServer::HitPlayer(SocketInfo* socketInfo, stringstream& recvStream)
{

}

void GameServer::ProcessPlayerDead(const int playerNumber)
{

}

void GameServer::HitZombie(SocketInfo* socketInfo, stringstream& recvStream)
{
	int zombieNumber = 0;
	recvStream >> zombieNumber;

	cout << "플레이어 " << socketInfo->number << "가 좀비 " << zombieNumber << "를 때렸습니다." << "\n";
	zombieMap[zombieNumber]->TakeDamage(100);
}

void GameServer::ProcessZombieDead(const int zombieNumber)
{
	cout << "좀비 " << zombieNumber << "가 죽었습니다.\n";

	EnterCriticalSection(&critsecZombieInfo);
	shared_ptr<Zombie> zombie = zombieMap[zombieNumber];
	zombieMap.erase(zombieNumber);
	LeaveCriticalSection(&critsecZombieInfo);

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::ZOMBIEDEAD) << "\n";
	sendStream << zombieNumber << "\n";
	Broadcast(sendStream);
}

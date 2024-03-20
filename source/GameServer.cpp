#include "GameServer.h"
#include "Player/Player.h"
#include "Zombie/Zombie.h"
#include <sstream>
#include <chrono>

using namespace std;

CRITICAL_SECTION						GameServer::critsecPlayerInfo;
unordered_map<int, SocketInfo*>			GameServer::playerSocketMap;
unordered_map<int, string>				GameServer::playerIDMap;
unordered_map<int, shared_ptr<Player>>	GameServer::playerMap;
int										GameServer::playerCount;
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

	packetCallbacks = vector<void(*)(SocketInfo*, stringstream&)>(static_cast<int>(EPacketType::PACKETTYPE_MAX));
	packetCallbacks[static_cast<int>(EPacketType::SIGNUP)] = SignUp;
	packetCallbacks[static_cast<int>(EPacketType::LOGIN)] = Login;
	packetCallbacks[static_cast<int>(EPacketType::SPAWNPLAYER)] = SpawnOtherPlayers;
	packetCallbacks[static_cast<int>(EPacketType::SYNCHPLAYER)] = SynchronizePlayerInfo;
	packetCallbacks[static_cast<int>(EPacketType::PLAYERINPUTACTION)] = BroadcastPlayerInputAction;
	packetCallbacks[static_cast<int>(EPacketType::WRESTLINGRESULT)] = ProcessPlayerWrestlingResult;

	InitializeCriticalSection(&critsecPlayerInfo);

	pathfinder.InitializePathFinder();

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
			stringstream sendStream;
			sendStream << static_cast<int>(EPacketType::SYNCHZOMBIE) << "\n";
			sendStream << zombieMap.size() << "\n";
			for (auto& kv : zombieMap)
			{
				kv.second->Update();
				if (packetFlag)
				{
					kv.second->SerializeData(sendStream);
				}
			}
			if (packetFlag)
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
	//for (int i = 0; i < maxZombieCount; i++)
	//{
	//	
	//}

	Vector3D location{ 800,-1000,97 };
	Rotator rotation{ 0,120,0 };

	zombieMap[0] = make_shared<Zombie>(0);
	zombieMap[0]->SetLocation(location);
	zombieMap[0]->SetRotation(rotation);

	location.X = 1100;
	location.Y = 900;
	rotation.yaw = -120;

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

	playerMap[socketInfo->number] = make_shared<Player>(socketInfo->number);
	shared_ptr<Player> playerPtr = playerMap[socketInfo->number];
	playerPtr->RegisterBroadcastCallback(ProcessPlayerWrestlingStart);
	// 방금 접속한 플레이어의 정보를 역직렬화
	playerPtr->DeserializeData(recvStream);

	// 방금 접속한 플레이어의 정보를 기존 플레이어들에게 보낼 스트림에 저장 
	stringstream newPlayerInfoStream;
	newPlayerInfoStream << static_cast<int>(EPacketType::SPAWNPLAYER) << "\n";
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
				zombieMap[number]->AddPlayerToInRangeMap(player);
			}
			break;
		}
		case PIBTC::ZombiesOutRange:
		{
			for (int number : player->GetZombiesOutRange())
			{
				zombieMap[number]->RemoveInRangePlayer(player->GetNumber());
			}
			break;
		}
		case PIBTC::ZombieAttackResult:
		{
			zombieMap[player->GetZombieNumberAttackedBy()]->ChangeState();
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

void GameServer::Broadcast(stringstream& sendStream, const int skipNumber)
{
	for (auto& info : playerSocketMap)
	{
		if (skipNumber != -1 && info.first == skipNumber) continue;
		Send(info.second, sendStream);
	}
}

#include "GameServer.h"
#include <sstream>
#include <chrono>

CRITICAL_SECTION					GameServer::critsecPlayerInfo;
unordered_map<int, SocketInfo*>		GameServer::playerSocketMap;
PlayerInfoSetEx						GameServer::playerInfoSetEx;
int									GameServer::playerCount;
ZombieInfoSet					GameServer::zombieInfoSet;
unordered_map<int, Zombie>			GameServer::zombieMap;

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
	packetCallbacks[static_cast<int>(EPacketType::PLAYERINPUTACTION)] = BroadcastPlyerInputAction;

	InitializeCriticalSection(&critsecPlayerInfo);

	pathfinder.InitializePathFinder();

	return true;
}

void GameServer::ZombieThread()
{
	InitializeZombieInfo();

	while (1)
	{
		auto start = std::chrono::high_resolution_clock::now();
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
				kv.second.Update();
				if (packetFlag)
				{
					sendStream << kv.first << "\n";
					sendStream << kv.second.GetZombieInfo() << "\n";
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

		//auto stop = std::chrono::high_resolution_clock::now();
		//auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
		//std::cout << "Time taken by function: " << duration.count() << " milliseconds" << std::endl;
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

	ZombieInfo info;

	info.characterInfo.velocityX = 0;
	info.characterInfo.velocityY = 0;
	info.characterInfo.velocityZ = 0;

	info.characterInfo.location.X = 800;
	info.characterInfo.location.Y = -1000;
	info.characterInfo.location.Z = 97;
	info.characterInfo.yaw = 120;

	zombieMap[0].SetZombieInfo(info);

	info.characterInfo.location.X = 1100;
	info.characterInfo.location.Y = 900;
	info.characterInfo.yaw = -120;

	//zombieMap[1].SetZombieInfo(info);
}

void GameServer::HandleDisconnectedClient(SocketInfo* socketInfo)
{
	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::PLAYERDISCONNECTED) << "\n";
	sendStream << socketInfo->number << "\n";
	sendStream << playerInfoSetEx.playerIDMap[socketInfo->number] << "\n";

	cout << "[Log] : " << socketInfo->number << "번 클라이언트 (ID : " << playerInfoSetEx.playerIDMap[socketInfo->number]  << ") 접속 종료\n";

	playerSocketMap.erase(socketInfo->number);					
	playerInfoSetEx.playerIDMap.erase(socketInfo->number);		
	playerInfoSetEx.characterInfoMap.erase(socketInfo->number);	

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
	playerInfoSetEx.playerIDMap[playerCount] = id;
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
	stringstream sendStream1;
	sendStream1 << static_cast<int>(EPacketType::SPAWNPLAYER) << "\n";
	playerInfoSetEx.OutputStreamWithID(sendStream1);

	// 방금 접속한 플레이어의 정보 추가
	recvStream >> playerInfoSetEx.characterInfoMap[socketInfo->number];

	// 방금 접속한 플레이어의 정보를 기존 플레이어들에게 보낼 스트림에 저장 
	PlayerInfoSetEx newPlayerInfoSet;
	newPlayerInfoSet.playerIDMap[socketInfo->number] = playerInfoSetEx.playerIDMap[socketInfo->number];
	newPlayerInfoSet.characterInfoMap[socketInfo->number] = playerInfoSetEx.characterInfoMap[socketInfo->number];

	stringstream sendStream2;
	sendStream2 << static_cast<int>(EPacketType::SPAWNPLAYER) << "\n";
	newPlayerInfoSet.OutputStreamWithID(sendStream2);	

	// 방금 접속한 플레이어에게 기존의 플레이어들 정보 전송
	Send(socketInfo, sendStream1);

	// 기존의 플레이어들에게 방금 접속한 플레이어 정보 전송
	Broadcast(sendStream2, socketInfo->number);
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::SynchronizePlayerInfo(SocketInfo* socketInfo, stringstream& recvStream)
{
	EnterCriticalSection(&critsecPlayerInfo);
	recvStream >> playerInfoSetEx.characterInfoMap[socketInfo->number];
	LeaveCriticalSection(&critsecPlayerInfo);

	PlayerInfo* info = &playerInfoSetEx.characterInfoMap[socketInfo->number];
	// 좀비의 타겟을 이 플레이어로 지정하고 상태 변경 및 이동 시키기
	for (int number : info->zombiesWhoSawMe)
	{
		zombieMap[number].SetTargetNumber(socketInfo->number);
		zombieMap[number].SetTarget(&info->characterInfo);
		zombieMap[number].ChangeState();
	}

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::SYNCHPLAYER) << "\n";
	sendStream << playerInfoSetEx << "\n";
	Send(socketInfo, sendStream);
}

void GameServer::BroadcastPlyerInputAction(SocketInfo* socketInfo, stringstream& recvStream)
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

void GameServer::Broadcast(stringstream& sendStream, const int skipNumber)
{
	for (auto& info : playerSocketMap)
	{
		if (skipNumber != -1 && info.first == skipNumber) continue;
		Send(info.second, sendStream);
	}
}

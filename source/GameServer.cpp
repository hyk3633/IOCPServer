#include "GameServer.h"
#include <sstream>

CRITICAL_SECTION GameServer::critsecPlayerInfo;
unordered_map<int, SocketInfo*> GameServer::playerSocketMap;
PlayerInfoSetEx GameServer::playerInfoSetEx;
int GameServer::playerCount;

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
	packetCallbacks[static_cast<int>(EPacketType::SYNCH)] = SynchronizePlayerInfo;

	InitializeCriticalSection(&critsecPlayerInfo);

	return true;
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
	playerInfoSetEx.playerInfoMap.erase(socketInfo->number);	
	for (auto& info : playerSocketMap)
	{
		if (info.first == socketInfo->number) continue;
		Send(info.second, sendStream);
	}
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
	PlayerInfo newPlayerInfo;
	recvStream >> newPlayerInfo;
	playerInfoSetEx.playerInfoMap[socketInfo->number] = newPlayerInfo;

	// 방금 접속한 플레이어의 정보를 기존 플레이어들에게 보낼 스트림에 저장 
	PlayerInfoSetEx newPlayerInfoSet;
	newPlayerInfoSet.playerIDMap[socketInfo->number] = playerInfoSetEx.playerIDMap[socketInfo->number];
	newPlayerInfoSet.playerInfoMap[socketInfo->number] = newPlayerInfo;

	stringstream sendStream2;
	sendStream2 << static_cast<int>(EPacketType::SPAWNPLAYER) << "\n";
	newPlayerInfoSet.OutputStreamWithID(sendStream2);

	LeaveCriticalSection(&critsecPlayerInfo);

	// 방금 접속한 플레이어에게 기존의 플레이어들 정보 전송
	Send(socketInfo, sendStream1);

	// 기존의 플레이어들에게 방금 접속한 플레이어 정보 전송
	for (auto& info : playerSocketMap)
	{
		if (info.first == socketInfo->number) continue;
		Send(info.second, sendStream2);
	}
}

void GameServer::SynchronizePlayerInfo(SocketInfo* socketInfo, stringstream& recvStream)
{
	EnterCriticalSection(&critsecPlayerInfo);
	recvStream >> playerInfoSetEx.playerInfoMap[socketInfo->number];
	LeaveCriticalSection(&critsecPlayerInfo);

	stringstream sendStream;
	sendStream << static_cast<int>(EPacketType::SYNCH) << "\n";
	sendStream << playerInfoSetEx << "\n";
	Send(socketInfo, sendStream);
}

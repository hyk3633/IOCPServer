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
	cout << "[Log] : " << socketInfo->number << "�� Ŭ���̾�Ʈ (ID : " << playerInfoSetEx.playerIDMap[socketInfo->number]  << ") ���� ����\n";
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

	cout << "[Log] : ID " << id << " �α��� ��û -> ���(" << isLoginSuccess << ")\n";

	EnterCriticalSection(&critsecPlayerInfo);
	playerInfoSetEx.playerIDMap[playerCount] = id;
	playerSocketMap[playerCount] = socketInfo;
	socketInfo->number = playerCount;
	playerCount++;
	LeaveCriticalSection(&critsecPlayerInfo);
}

void GameServer::SpawnOtherPlayers(SocketInfo* socketInfo, stringstream& recvStream)
{
	cout << "[Log] : " << socketInfo->number << "�� Ŭ���̾�Ʈ ���� �ʿ� ����\n";

	EnterCriticalSection(&critsecPlayerInfo);

	// ���� �÷��̾���� ������ ��� ������ �÷��̾�� ���� ��Ʈ���� ����
	stringstream sendStream1;
	sendStream1 << static_cast<int>(EPacketType::SPAWNPLAYER) << "\n";
	playerInfoSetEx.OutputStreamWithID(sendStream1);

	// ��� ������ �÷��̾��� ���� �߰�
	PlayerInfo newPlayerInfo;
	recvStream >> newPlayerInfo;
	playerInfoSetEx.playerInfoMap[socketInfo->number] = newPlayerInfo;

	// ��� ������ �÷��̾��� ������ ���� �÷��̾�鿡�� ���� ��Ʈ���� ���� 
	PlayerInfoSetEx newPlayerInfoSet;
	newPlayerInfoSet.playerIDMap[socketInfo->number] = playerInfoSetEx.playerIDMap[socketInfo->number];
	newPlayerInfoSet.playerInfoMap[socketInfo->number] = newPlayerInfo;

	stringstream sendStream2;
	sendStream2 << static_cast<int>(EPacketType::SPAWNPLAYER) << "\n";
	newPlayerInfoSet.OutputStreamWithID(sendStream2);

	LeaveCriticalSection(&critsecPlayerInfo);

	// ��� ������ �÷��̾�� ������ �÷��̾�� ���� ����
	Send(socketInfo, sendStream1);

	// ������ �÷��̾�鿡�� ��� ������ �÷��̾� ���� ����
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

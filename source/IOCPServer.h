#pragma once

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <map>
#include <vector>
#include <iostream>
#include <thread>
#include "Define.h"
#include "DBConnector.h"

using namespace std;

struct SocketInfo
{
	WSAOVERLAPPED overlapped;	// overlapped 구조체
	WSABUF wsaBuf;				// io 작업버퍼
	SOCKET socket;
	char msgBuf[PACKET_SIZE];
	int number;
};

class IOCPServer
{
public:

	IOCPServer();

	virtual ~IOCPServer();

	virtual bool InitializeServer();

	virtual void StartServer();

	virtual void WorkerThread();

protected:

	bool CreateWorkerThreads();

	virtual bool CreateZombieThread() = 0;

	void AccepterThread();

	virtual void HandleDisconnectedClient(SocketInfo* socketInfo) = 0;

	static void Send(SocketInfo* socketInfo, stringstream& sendStream);

	static void Recv(SocketInfo* socketInfo);

protected:

	SOCKET listenSocket;

	HANDLE iocpHandle;

	thread accepterThread;

	vector<HANDLE*> workerThreads;

	vector<void(*)(SocketInfo*, stringstream&)> packetCallbacks;

	SocketInfo* socketInfo;

	int threadCount;

	static DBConnector* dbConnector;

};
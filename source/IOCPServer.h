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
};

class IOCPServer
{
public:

	IOCPServer();

	virtual ~IOCPServer();

	bool InitializeServer();

	virtual void StartServer();

	virtual void WorkerThread();

protected:

	bool CreateWorkerThreads();

	void AccepterThread();

	static void Send(SocketInfo* socketInfo, stringstream& sendStream);

	static void Recv(SocketInfo* socketInfo);

	static void SignUp(SocketInfo* , stringstream&);

	static void Login(SocketInfo* , stringstream&);

protected:

	SOCKET listenSocket;

	HANDLE iocpHandle;

	thread accepterThread;

	vector<HANDLE*> workerThreads;

	vector<void(*)(SocketInfo*, stringstream&)> packetCallbacks;

	SocketInfo* socketInfo;

	int threadCount;

private:

	static DBConnector* dbConnector;

};
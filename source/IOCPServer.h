#pragma once

#pragma comment(lib, "ws2_32.lib")
#include <WinSock2.h>
#include <map>
#include <vector>
#include <iostream>
#include <thread>
#include "Define.h"

using namespace std;

struct SocketInfo
{
	WSAOVERLAPPED overlapped;	// overlapped ����ü
	WSABUF wsaBuf;				// io �۾�����
	SOCKET socket;
	int clientNumber;
};

class IOCPServer
{
public:

	IOCPServer();

	virtual ~IOCPServer();

	bool InitializeServer();

	virtual void StartServer();

	bool CreateWorkerThreads();

	virtual void WorkerThread();

	void AccepterThread();

protected:

	SOCKET listenSocket;

	HANDLE iocpHandle;

	thread accepterThread;

	vector<HANDLE*> workerThreads;

	SocketInfo* socketInfo;

	int threadCount;

	char dataBuf[4096];

	// �׽�Ʈ��
	int tempNumber = 0;

};
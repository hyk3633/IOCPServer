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
	WSAOVERLAPPED overlapped;	// overlapped ����ü
	WSABUF wsaBuf;				// io �۾�����
	SOCKET socket;
	char msgBuf[PACKET_SIZE];
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

	// �׽�Ʈ��
	int tempNumber = 0;

private:

	DBConnector& dbConnector = DBConnector::GetInstance();

};
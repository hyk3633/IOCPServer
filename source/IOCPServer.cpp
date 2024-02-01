#include "IOCPServer.h"
#include <sstream>


unsigned int WINAPI WorkerThreadStart(LPVOID param)
{
	IOCPServer* iocpEvent = reinterpret_cast<IOCPServer*>(param);
	iocpEvent->WorkerThread();
	return 0;
}

IOCPServer::IOCPServer()
{

}

IOCPServer::~IOCPServer()
{
	WSACleanup();
	if (socketInfo)
	{
		delete[] socketInfo;
		socketInfo = NULL;
	}
}

bool IOCPServer::InitializeServer()
{
	// WinSock 초기화
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		cout << "[Error] : Failed to initializing WinSock!\n";
		return false;
	}

	// 리스닝 소켓 생성
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "[Error] : Failed to creating a socket!\n";
		WSACleanup();
		return false;
	}

	// 소켓에 IP 주소 및 포트 번호를 바인딩 하기 위한 IPv4 패킷용 주소 구조체
	sockaddr_in serverAddr{};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(SERVER_PORT);
	serverAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	// 소켓 바인딩
	result = bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr));
	if (result != 0)
	{
		cout << "[Error] : Failed to binding socket!\n";
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	// 소켓 리스닝 시작
	result = listen(listenSocket, 5);
	if (result != 0)
	{
		cout << "[Error] : Failed to listening a socket!\n";
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}
	cout << "[Log] Successfully initialzed server!!\n";

	return true;
}

void IOCPServer::StartServer()
{
	// Completion port 객체 생성
	iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (iocpHandle == NULL)
	{
		cout << "[Error] : Failed to creating completion port!\n";
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	// Worker thread 생성
	if (!CreateWorkerThreads())
	{
		return;
	}

	cout << "[Log] : Server started!\n";

	// Accepter thread 생성
	accepterThread = thread([this]() { AccepterThread(); });
	accepterThread.join();
	closesocket(listenSocket);
}

bool IOCPServer::CreateWorkerThreads()
{
	unsigned int threadId;
	SYSTEM_INFO sysInfo;
	GetSystemInfo(&sysInfo);
	threadCount = sysInfo.dwNumberOfProcessors * 2;

	for (int i = 0; i < threadCount; i++)
	{
		workerThreads.push_back((HANDLE*)_beginthreadex(NULL, 0, &WorkerThreadStart, this, CREATE_SUSPENDED, &threadId));
		if (workerThreads[i] == NULL)
		{
			return false;
		}
		ResumeThread(workerThreads[i]);
	}

	return true;
}

void IOCPServer::WorkerThread()
{
	int result;
	DWORD recvBytes;
	SocketInfo* completionKey;
	SocketInfo* recvSocketInfo; // unique ptr?
	char buff[100];
	while (1)
	{
		result = GetQueuedCompletionStatus(iocpHandle, &recvBytes, (PULONG_PTR)&completionKey, (LPOVERLAPPED*)&recvSocketInfo, INFINITE);
		if (result == 0 || recvBytes == 0)
		{
			cout << "Client end connection\n";
			closesocket(recvSocketInfo->socket);
			free(recvSocketInfo);
			continue;
		}

		if (recvSocketInfo == nullptr) continue;

		int packetType;
		stringstream recvStream;

		recvStream << recvSocketInfo->wsaBuf.buf;
		recvStream >> packetType;
		ZeroMemory(&buff, 100);
		recvStream >> buff;

		if (static_cast<EPacketType>(packetType) == EPacketType::TEST)
		{
			cout << recvSocketInfo->clientNumber << " : 패킷 타입 TEST\n";
			cout << buff << "\n";
		}
		else if (static_cast<EPacketType>(packetType) == EPacketType::RECV)
		{
			cout << recvSocketInfo->clientNumber << " : 패킷 타입 RECV\n";
			cout << buff << "\n";
		}

		// 함수로 캡슐화 하기
		DWORD flags = 0;
		result = WSARecv(socketInfo->socket, &(socketInfo->wsaBuf), 1, (LPDWORD)&socketInfo, &flags, (LPWSAOVERLAPPED) & (socketInfo->overlapped), NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "[Error] : WSARecv failed -> " << WSAGetLastError();
		}
	}
}

void IOCPServer::AccepterThread()
{
	int result;
	SOCKET clientSocket;
	sockaddr_in clientAddr;
	int addrLen = sizeof(clientAddr);
	DWORD recvBytes = 0;
	DWORD flags = 0;

	while (1)
	{
		clientSocket = WSAAccept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen, NULL, NULL);
		if (clientSocket == INVALID_SOCKET)
		{
			cout << "[Error] : Failed to accept a socket!\n";
			closesocket(clientSocket);
			return;
		}

		socketInfo = new SocketInfo();
		socketInfo->socket = clientSocket;
		socketInfo->wsaBuf.len = 4096;
		socketInfo->wsaBuf.buf = dataBuf;
		socketInfo->clientNumber = tempNumber++;

		iocpHandle = CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (DWORD)socketInfo, 0);

		result = WSARecv(socketInfo->socket, &socketInfo->wsaBuf, 1, &recvBytes, &flags, &(socketInfo->overlapped), NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "[Error] : WSA_IO_PENDING ->" << WSAGetLastError();
			closesocket(clientSocket);
			return;
		}
	}
}

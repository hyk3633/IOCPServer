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
	// DB 초기화 및 연결
	if (dbConnector.Initialize())
	{
		if (dbConnector.Connect() == false) 
			return false;
	}
	else return false;

	// WinSock 초기화
	WSADATA wsaData;
	int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (result != 0)
	{
		cout << "[Error] : Failed to initializing WinSock!" << endl;
		return false;
	}

	// 리스닝 소켓 생성
	listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
	if (listenSocket == INVALID_SOCKET)
	{
		cout << "[Error] : Failed to creating a socket!" << endl;
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
		cout << "[Error] : Failed to binding socket!" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}

	// 소켓 리스닝 시작
	result = listen(listenSocket, 5);
	if (result != 0)
	{
		cout << "[Error] : Failed to listening a socket!" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return false;
	}
	cout << "[Log] Successfully initialzed server!!" << endl;

	return true;
}

void IOCPServer::StartServer()
{
	// Completion port 객체 생성
	iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 0);
	if (iocpHandle == NULL)
	{
		cout << "[Error] : Failed to creating completion port!" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	// Worker thread 생성
	if (!CreateWorkerThreads())
	{
		cout << "[Error] : Failed to creating worker threads!" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return;
	}

	cout << "[Log] : Server started!" << endl;

	// Accepter thread 생성
	accepterThread = thread([this]() { AccepterThread(); });

	accepterThread.join();
	closesocket(listenSocket);
	dbConnector.Close();
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

	while (1)
	{
		// IO 완료된 작업 꺼내오기
		result = GetQueuedCompletionStatus(iocpHandle, &recvBytes, (PULONG_PTR)&completionKey, (LPOVERLAPPED*)&recvSocketInfo, INFINITE);
		if (!result || !recvBytes)
		{
			if(!result) cout << "[Log] : Client end connection." << endl;
			closesocket(recvSocketInfo->socket);
			free(recvSocketInfo);
			continue;
		}

		if (recvSocketInfo == nullptr) continue;

		recvSocketInfo->wsaBuf.len = recvBytes;
		int packetType;
		stringstream recvStream;

		recvStream << recvSocketInfo->wsaBuf.buf;
		recvStream >> packetType;

		// 함수 포인터로
		if (static_cast<EPacketType>(packetType) == EPacketType::SIGNUP)
		{
			string id, pw;
			recvStream >> id >> pw;

			stringstream sendStream;
			sendStream << packetType << "\n";
			sendStream << dbConnector.PlayerSignUp(id, pw) << "\n";

			CopyMemory(recvSocketInfo->msgBuf, sendStream.str().c_str(), sendStream.str().length());
			recvSocketInfo->wsaBuf.buf = recvSocketInfo->msgBuf;
			recvSocketInfo->wsaBuf.len = sendStream.str().length();

			int nResult;
			DWORD	sendBytes;
			DWORD	dwFlags = 0;

			nResult = WSASend(recvSocketInfo->socket, &(recvSocketInfo->wsaBuf), 1, &sendBytes, dwFlags, NULL, NULL);
			if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				printf_s("[ERROR] WSASend 실패 : ", WSAGetLastError());
			}
		}
		else if (static_cast<EPacketType>(packetType) == EPacketType::LOGIN)
		{
			string id, pw;
			recvStream >> id >> pw;

			stringstream sendStream;
			sendStream << packetType << "\n";
			sendStream << dbConnector.PlayerLogin(id, pw) << "\n";

			CopyMemory(recvSocketInfo->msgBuf, sendStream.str().c_str(), sendStream.str().length());
			recvSocketInfo->wsaBuf.buf = recvSocketInfo->msgBuf;
			recvSocketInfo->wsaBuf.len = sendStream.str().length();

			int nResult;
			DWORD	sendBytes;
			DWORD	dwFlags = 0;

			nResult = WSASend(recvSocketInfo->socket, &(recvSocketInfo->wsaBuf), 1, &sendBytes, dwFlags, NULL, NULL);
			if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
			{
				printf_s("[ERROR] WSASend 실패 : ", WSAGetLastError());
			}
		}

		// 함수로 캡슐화 하기
		DWORD flags = 0;

		ZeroMemory(&(socketInfo->overlapped), sizeof(OVERLAPPED));
		ZeroMemory(socketInfo->msgBuf, PACKET_SIZE);
		socketInfo->wsaBuf.len = PACKET_SIZE;
		socketInfo->wsaBuf.buf = socketInfo->msgBuf;

		result = WSARecv(socketInfo->socket, &(socketInfo->wsaBuf), 1, (LPDWORD)&socketInfo, &flags, (LPWSAOVERLAPPED) & (socketInfo->overlapped), NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "[Error] : WSARecv failed -> " << WSAGetLastError() << endl;
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
		socketInfo->wsaBuf.len = PACKET_SIZE;
		socketInfo->wsaBuf.buf = socketInfo->msgBuf;
		socketInfo->clientNumber = tempNumber++;

		// iocp에 클라이언트 소켓 등록
		iocpHandle = CreateIoCompletionPort((HANDLE)clientSocket, iocpHandle, (DWORD)socketInfo, 0);

		cout << "[Log] : A new player has been connected.\n";

		// 비동기 recv 시작
		result = WSARecv(socketInfo->socket, &socketInfo->wsaBuf, 1, &recvBytes, &flags, &(socketInfo->overlapped), NULL);
		if (result == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		{
			cout << "[Error] : WSA_IO_PENDING ->" << WSAGetLastError();
			closesocket(clientSocket);
			return;
		}
	}
}

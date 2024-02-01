#include "IOCPServer.h"

int main()
{
	IOCPServer server;
	if (server.InitializeServer())
	{
		server.StartServer();
	}

	return 0;
}
#include "GameServer.h"

int main()
{
	GameServer server;
	if (server.InitializeServer())
	{
		server.StartServer();
	}

	return 0;
}
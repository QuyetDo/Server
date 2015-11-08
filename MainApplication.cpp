// G4.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Server.h"

int _tmain(int argc, _TCHAR* argv[])
{
	Server server;
	if (!server.setupServer("127.0.0.1", DEFAULT_PORT))
	{
		printf("Can't set up server");
		return 1;
	}
	server.run();
	getchar();
	return 0;
}


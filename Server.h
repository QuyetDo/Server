#ifndef SERVER_H
#define SERVER_H

#include "Type.h"
#include <winsock2.h>
#include <ws2tcpip.h>
#include <queue>

class Server
{
public:
	Server();
	bool setupServer(char* addr, unsigned short port);
	int run();

private:
	SOCKET m_listenSocket;
	unsigned int m_concurentNumber;
	std::queue<ClientConnection*> m_pendingClient;
private:
	static DWORD WINAPI handlePendingConnection(LPVOID lpParam);
	void finish() const;
};
#endif
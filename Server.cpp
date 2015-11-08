
#include "stdafx.h"
#include "Server.h"
#include "SessionHandler.h"
#include "ConnectedClientsManager.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
#define DEFAULT_BUFLEN 512

namespace{
	const char* DISCONN = "DISCONN";
};


Server::Server()
{
	m_concurentNumber = 5000;
}


///////////////////////////////////////////////////////////////////////////////////////////
//! Setup an TCP IP server with input address and port
///////////////////////////////////////////////////////////////////////////////////////////
bool Server::setupServer(char* addr, unsigned short port)
{
	if (addr == NULL)
	{
		return false;
	}
	WSADATA wsaData;
	int iResult;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return false;
	}

	// Create a SOCKET for connecting to server
	m_listenSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_listenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return false;
	}

	SOCKADDR_IN target;
	target.sin_family = AF_INET;
	target.sin_port = htons(port);
	inet_pton(AF_INET, addr, &target.sin_addr);

	// Setup the TCP listening socket
	if (bind(m_listenSocket, (SOCKADDR *)&target, sizeof(target)) == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
//! Starts the server
//! It accepts the connection from clients and adds them to pending list
///////////////////////////////////////////////////////////////////////////////////////////
int Server::run()
{
	DWORD   handlingThreadId;
	HANDLE handlingThread = CreateThread(NULL, 0, &Server::handlePendingConnection, this, 0, &handlingThreadId);

	if (listen(m_listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		finish();
		return 1;
	}
	while (1)
	{
		// Accept a client socket
		ClientConnection* client = new ClientConnection();
		client->client_Socket = accept(m_listenSocket, (sockaddr*)&client->client_Addr, NULL);
		if (client->client_Socket == INVALID_SOCKET) {
			printf("accept failed with error: %d\n", WSAGetLastError());
			finish();
			return 1;
		}
		m_pendingClient.push(client);
	}
	finish();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
//! Checks the pending clients and create a session handler to control it
//! if the resource is available
///////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI Server::handlePendingConnection(LPVOID lpParam)
{
	if( lpParam == NULL )
	{
		return 1;
	}

	Server* server = (Server*)lpParam;
	ConnectedClientsManager& clientMgr = ConnectedClientsManager::getInstance();
	while (true)
	{
		while (!server->m_pendingClient.empty() && server->m_concurentNumber >= clientMgr.getConnClientNumber())
		{
			ClientConnection* connectData = server->m_pendingClient.front();
			if (connectData == NULL)
			{
				// No pending client
				break;
			}
			server->m_pendingClient.pop();

			ConnectionHandler* connection = new ConnectionHandler(connectData);
			connection->run();
		}
		Sleep(500);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//! This function should be called to close the server
///////////////////////////////////////////////////////////////////////////////////////////
void Server::finish() const
{
	// shutdown the connection since we're done
	closesocket(m_listenSocket);
	WSACleanup();
}
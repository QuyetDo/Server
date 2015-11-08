#include"stdafx.h"
#include"SessionHandler.h"
#include"ConnectedClientsManager.h"


namespace{
#define DEFAULT_BUFLEN 512
	const unsigned short DEFAULT_PORT = 27015;
	const char* DISCONN = "DISCONN";
	const char* KEEP_ALIVE = "KEEP_ALIVE";
	const char* CONN = "CONN";
};

ConnectionHandler::ConnectionHandler(ClientConnection* client) : m_client(client)
{
	m_isAlive = false;
	m_hasJoined = false;
	m_loginTime = NULL;
}

ConnectionHandler::~ConnectionHandler()
{
	finishSession();
	free(m_client);
	if (m_loginTime != NULL)
	{
		free(m_loginTime);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////
//! Generates message to broadcast to all currently connected clients depend on iput type
///////////////////////////////////////////////////////////////////////////////////////////
char* ConnectionHandler::generateBroadcastMessage(MessageType type)
{
	time_t rawtime(0);
	struct tm timeinfo;

	time(&rawtime);
	localtime_s(&timeinfo, &rawtime);

	// Gets client address
	char str[INET_ADDRSTRLEN];
	sockaddr_in* ipV4 = (sockaddr_in*)&m_client->client_Addr;
	inet_ntop(AF_INET, &ipV4->sin_addr, str, INET_ADDRSTRLEN);

	char* logoutTime;
	char* message = NULL;
	switch (type)
	{
	case MT_CONN:
		m_loginTime = (char*)malloc(40 * sizeof(char));
		sprintf_s(m_loginTime, 40, "%.2d:%.2d:%.2d on %d / %.2d / %4d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon, timeinfo.tm_year + 1900);
		message = (char*)malloc(110 * sizeof(char));
		sprintf_s(message, 110, "New client connected from %s:%d at %s", str, ipV4->sin_port, m_loginTime);
		break;
	case MT_DISCONN:
		logoutTime = (char*)malloc(40 * sizeof(char));
		sprintf_s(logoutTime, 40, "%.2d:%.2d:%.2d on %d / %.2d / %4d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon, timeinfo.tm_year + 1900);
		message = (char*)malloc(250 * sizeof(char));
		sprintf_s(message, 250, "The client disconnected from %s:%d at %s that was connected at %s", str, ipV4->sin_port, logoutTime, m_loginTime);
		free(logoutTime);
		break;
	default:
		break;
	}
	return message;
}

///////////////////////////////////////////////////////////////////////////////////////////
//! Decode received message and return its type
///////////////////////////////////////////////////////////////////////////////////////////
MessageType ConnectionHandler::decodeRecvMessage(char* message)
{
	printf("content received: %s\n", message);

	if (!strcmp(DISCONN, message))
	{
		// Client want to close session
		return MT_DISCONN;
	}

	if (!strcmp(CONN, message))
	{
		addNewClient();
		return MT_CONN;
	}
	return MT_ALIVE;

}

///////////////////////////////////////////////////////////////////////////////////////////
//! Receive message from client
///////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI ConnectionHandler::recvMessage(LPVOID lpParam)
{
	if (lpParam == NULL)
	{
		return 1;
	}
	int recvResult = 0;
	char recvbuf[DEFAULT_BUFLEN];
	int recvbuflen = DEFAULT_BUFLEN;
	ConnectionHandler* owner = (ConnectionHandler*)lpParam;
	SOCKET *client = &owner->m_client->client_Socket;
	do {
		memset(recvbuf, NULL, DEFAULT_BUFLEN);
		recvResult = recv(*client, recvbuf, recvbuflen, 0);
		if (recvResult > 0) {
			if (owner->decodeRecvMessage(recvbuf) == MT_DISCONN)
			{
				break;
			}
			// Reset alive status
			owner->m_isAlive = true;
		}
		else if (recvResult == 0)
			printf("Session finished...\n");
		else  {
			printf("recv failed with error: %d\n", WSAGetLastError());
		}

	} while (recvResult > 0);

	owner->finishSession();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
//! Adds this client to the connected list
//! and broadcast to all about its connection
///////////////////////////////////////////////////////////////////////////////////////////
void ConnectionHandler::addNewClient()
{
	m_hasJoined = true;
	ConnectedClientsManager& clientMgr = ConnectedClientsManager::getInstance();
	clientMgr.addNewClient(&m_client->client_Socket);

	char* message = generateBroadcastMessage(MT_CONN);
	clientMgr.sendBroadcastMessage(message, &m_client->client_Socket);
	free(message);

	DWORD   aliveId;
	HANDLE timeoutThread = CreateThread(NULL, 0, &ConnectionHandler::checkTimeOut, this, 0, &aliveId);

}

///////////////////////////////////////////////////////////////////////////////////////////
//! Starts a connection session
//! It creates threads to receive message from client and check time out.
///////////////////////////////////////////////////////////////////////////////////////////
void ConnectionHandler::run()
{
	if (m_client->client_Socket == INVALID_SOCKET)
	{
		return;
	}

	DWORD   threadId;
	HANDLE thread = CreateThread(NULL, 0, &ConnectionHandler::recvMessage, this, 0, &threadId);
}

///////////////////////////////////////////////////////////////////////////////////////////
//! This function checks alive timeout and session timeout
//! It also sends the connected time value to client secondly
///////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI ConnectionHandler::checkTimeOut(LPVOID lpParam)
{
	if (lpParam == NULL)
	{
		return 1;
	}

	ConnectionHandler* owner = ((ConnectionHandler*)lpParam);
	int aliveCount = 0;
	int sessionCount = 0;
	char* connectedTime = (char*)malloc(30);
	
	while (aliveCount < 30 && sessionCount < 500)
	{
		aliveCount++;
		sessionCount++;
		if (owner->m_client->client_Socket == INVALID_SOCKET)
		{
			//Session has been finished
			return 0;
		}

		if (owner->m_isAlive)
		{
			aliveCount = 0;
			owner->m_isAlive = false;
		}
		Sleep(1000);
		memset(connectedTime, NULL, 30);
		sprintf_s(connectedTime, 30, "You have connected for %ds", sessionCount);
		send(owner->m_client->client_Socket, connectedTime, (int)strlen(connectedTime), 0);
	}
	
	// Alive message time out has been reached
	owner->finishSession();
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////
//! This function should be called to close current session
//! It send a disconnected message to all currently connected clients
//! then shudown current session
///////////////////////////////////////////////////////////////////////////////////////////
void ConnectionHandler::finishSession()
{
	if (m_client->client_Socket == INVALID_SOCKET)
	{
		//Session has been finished
		return;
	}

	if (m_hasJoined)
	{
		ConnectedClientsManager::getInstance().removeExistedClient(&m_client->client_Socket);

		char* message = generateBroadcastMessage(MT_DISCONN);
		ConnectedClientsManager::getInstance().sendBroadcastMessage(message, &m_client->client_Socket);
		free(message);
	}

	int result = shutdown(m_client->client_Socket, SD_SEND);
	if (result == SOCKET_ERROR) {
		printf("shutdown connection failed with error: %d\n", WSAGetLastError());
	}
	m_client->client_Socket = INVALID_SOCKET;
}
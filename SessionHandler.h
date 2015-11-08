#ifndef SESSION_HANDLER_H
#define SESSION_HANDLER_H

#include "Type.h"
#include <thread>
//#include <list>
#include <winsock2.h>
#include <ws2tcpip.h>
class ConnectionHandler
{
private:
	ClientConnection* m_client;
	bool m_isAlive;
	bool m_hasJoined;
	char* m_loginTime;

public:
	ConnectionHandler(ClientConnection* client);
	~ConnectionHandler();

	void run();
	//static const std::list<SOCKET*>& getConnectedClientList();
	
private:
	static DWORD WINAPI recvMessage(LPVOID lpParam);
	char* generateBroadcastMessage(MessageType type);
	static DWORD WINAPI checkTimeOut(LPVOID lpParam);
	MessageType decodeRecvMessage(char* message);
	void addNewClient();
	void finishSession();
	//void sendBroadcastMessage();
};

#endif
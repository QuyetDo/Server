#ifndef CONNECTED_CLIENT_MANAGER_H
#define CONNECTED_CLIENT_MANAGER_H

#include <list>
#include <winsock2.h>
#include <ws2tcpip.h>
class ConnectedClientsManager
{
public:
	~ConnectedClientsManager();
	static ConnectedClientsManager& getInstance();

	void addNewClient(SOCKET* client);
	bool removeExistedClient(SOCKET* client);
	unsigned int getConnClientNumber() const;
	void sendBroadcastMessage(const char* message, const SOCKET* newClient) const;

private:
	std::list<SOCKET*> m_connectedClients;

private:
	ConnectedClientsManager();

};

#endif
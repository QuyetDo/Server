#include "stdafx.h"
#include "ConnectedClientsManager.h"

ConnectedClientsManager::ConnectedClientsManager()
{
}

ConnectedClientsManager::~ConnectedClientsManager()
{
}

///////////////////////////////////////////////////////////////////////////////////////////
ConnectedClientsManager& ConnectedClientsManager::getInstance()
{
	static ConnectedClientsManager connClientMgr;
	return connClientMgr;
}

///////////////////////////////////////////////////////////////////////////////////////////
void ConnectedClientsManager::addNewClient(SOCKET* client)
{
	m_connectedClients.push_back(client);
}

///////////////////////////////////////////////////////////////////////////////////////////
bool ConnectedClientsManager::removeExistedClient(SOCKET* client)
{
	m_connectedClients.remove(client);
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////
unsigned int ConnectedClientsManager::getConnClientNumber() const
{
	return m_connectedClients.size();
}

///////////////////////////////////////////////////////////////////////////////////////////
void ConnectedClientsManager::sendBroadcastMessage(const char* message, const SOCKET* newClient) const
{
	if (message == NULL)
	{
		return;
	}
	std::list<SOCKET*> connectedClients = m_connectedClients;
	while (!connectedClients.empty())
	{
		SOCKET* client = connectedClients.front();
		connectedClients.pop_front();
		if (client != newClient)
		{
			send(*client, message, strlen(message), 0);
		}
	}
}
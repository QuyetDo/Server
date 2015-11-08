#ifndef TYPE_H
#define TYPE_H

#include <winsock2.h>
const unsigned short DEFAULT_PORT = 27015;

typedef struct ClientConnection
{
	SOCKET client_Socket;
	struct sockaddr  client_Addr;
} ClientConnection;

enum MessageType
{
	MT_CONN,
	MT_ALIVE,
	MT_DISCONN
};

#endif
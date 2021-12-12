#pragma once
#include "CommonMacro.h"

#include <thread>
#include <WinSock2.h>

START_NAMESPACE(NetworkLib)

class Server
{
public:
	Server(short port, int recvBufferSize);
	void start();

	virtual void onAccept(SOCKET clientSocket) = 0;
	virtual void onRecv(SOCKET clientSocket, char *buffer, int recvSize) = 0;
	virtual void onClose(SOCKET clientSocket) = 0;

	std::thread *getLoopThread();

private:
	short port;
	int recvBufferSize;
	std::thread* loopThread;

	WSAData wsaData;
	SOCKET serverSocket;

	int socketCount;
	SOCKET sockets[WSA_MAXIMUM_WAIT_EVENTS];
	WSAEVENT socketEvents[WSA_MAXIMUM_WAIT_EVENTS];

	void loop();

	void _onAccept(int index);
	void _onRecv(int index);
	void _onClose(int index);
};

END_NAMESPACE
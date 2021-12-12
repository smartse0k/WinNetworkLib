#include "CommonMacro.h"
#include "Server.h"

START_NAMESPACE(NetworkLib)

Server::Server(short port, int recvBufferSize)
{
	this->port = port;
	this->recvBufferSize = recvBufferSize;

	int ret = WSAStartup(MAKEWORD(2, 2), &this->wsaData);
}

void Server::start()
{
	sockaddr_in socketAddress;
	socketAddress.sin_family = AF_INET;
	socketAddress.sin_port = htons(this->port);
	socketAddress.sin_addr.s_addr = htonl(INADDR_ANY);

	this->serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	bind(this->serverSocket, (sockaddr *)&socketAddress, sizeof(sockaddr_in));
	listen(this->serverSocket, SOMAXCONN);

	WSAEVENT serverSocketEvent = WSACreateEvent();
	WSAEventSelect(this->serverSocket, serverSocketEvent, FD_ACCEPT);

	this->sockets[0] = this->serverSocket;
	this->socketEvents[0] = serverSocketEvent;

	this->socketCount = 1;

	this->loopThread = new std::thread([this]() {
		loop();
	});
}

void Server::loop()
{
	while (true) {
		DWORD ret = WSAWaitForMultipleEvents(this->socketCount, this->socketEvents, FALSE, WSA_INFINITE, TRUE);
		
		if (ret == WSA_WAIT_IO_COMPLETION) {
			// TODO
			continue;
		} else if (ret == WSA_WAIT_TIMEOUT) {
			// TODO
			continue;
		} else if (ret < WSA_WAIT_EVENT_0 && ret >= WSA_WAIT_EVENT_0 + WSA_MAXIMUM_WAIT_EVENTS) {
			// TODO
			continue;
		}

		int index = ret - WSA_WAIT_EVENT_0;

		WSANETWORKEVENTS networkEvents;
		WSAEnumNetworkEvents(this->sockets[index], this->socketEvents[index], &networkEvents);

		if (networkEvents.lNetworkEvents & FD_ACCEPT) {
			this->_onAccept(index);
		}

		if (networkEvents.lNetworkEvents & FD_READ) {
			this->_onRecv(index);
		}

		if (networkEvents.lNetworkEvents & FD_CLOSE) {
			this->_onClose(index);
		}
	}
}

std::thread *Server::getLoopThread()
{
	return this->loopThread;
}

void Server::_onAccept(int index)
{
	sockaddr_in socketAddress;
	int size = sizeof(socketAddress);
	SOCKET clientSocket = accept(this->sockets[index], (sockaddr*)&socketAddress, &size);

	WSAEVENT clientSocketEvent = WSACreateEvent();
	WSAEventSelect(clientSocket, clientSocketEvent, FD_READ | FD_CLOSE);

	this->sockets[this->socketCount] = clientSocket;
	this->socketEvents[this->socketCount] = clientSocketEvent;

	this->socketCount++;

	this->onAccept(clientSocket);
}

void Server::_onRecv(int index)
{
	int bufferSize = this->recvBufferSize;

	char* buffer = new char[bufferSize];
	int recvSize = recv(this->sockets[index], buffer, bufferSize, 0);

	this->onRecv(this->sockets[index], buffer, recvSize);

	delete[] buffer;
}

void Server::_onClose(int index)
{
	this->onClose(this->sockets[index]);

	for (int i = index; i < this->socketCount - 1; i++) {
		this->sockets[i] = this->sockets[i + 1];
	}

	this->socketCount--;

	closesocket(this->sockets[index]);
	WSACloseEvent(this->socketEvents[index]);
}

END_NAMESPACE
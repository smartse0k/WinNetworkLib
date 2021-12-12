#include <iostream>

#include "Server.h"

class EchoServer : public NetworkLib::Server {
public:
	EchoServer(short port) : Server(port, 1024) {}

	void onAccept(SOCKET clientSocket) override {
		std::cout << "Accepted." << std::endl;
	}

	void onRecv(SOCKET clientSocket, char *buffer, int bufferSize) override {
		std::cout << "Received " << bufferSize << " bytes." << std::endl;

		char* recvBuffer = new char[bufferSize + 1];
		memcpy(recvBuffer, buffer, bufferSize);
		recvBuffer[bufferSize] = 0;

		std::cout << "Client: " << recvBuffer << std::endl;

		send(clientSocket, recvBuffer, bufferSize, 0);
	}

	void onClose(SOCKET clientSocket) override {
		std::cout << "Closed." << std::endl;
	}
};

int main(void) {
	std::cout << "Start" << std::endl;

	EchoServer server(11111);
	server.start();

	std::cout << "Started" << std::endl;

	server.getLoopThread()->join();

	return 0;
}

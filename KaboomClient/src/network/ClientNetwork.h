#pragma once

#include <WinSock2.h>

class ClientNetwork {
public:
    ClientNetwork();
    ~ClientNetwork();

    void connectToServer(const std::string &serverAddress, const int &serverPort);
	void disconnectFromServer();

    int receivePackets(char *, int bufSize);
    void sendMessage(char *message, int size) const;

	inline bool isConnected() const
	{
		return _connected;
	}

private:
    SOCKET clientSocket;
	bool _connected;
};
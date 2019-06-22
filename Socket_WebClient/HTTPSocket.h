#pragma once
#include "HTTPData.h"

class HTTPSocket
{
public:
	HTTPSocket(int sock);
	HTTPSocket(string host);
	bool isOpened();
	bool isClosed();
	bool Receive(HTTPData* data);
	bool Send(HTTPData* data);
	virtual ~HTTPSocket();
private:
	int socket;
	bool is_open;
	bool is_close;
};


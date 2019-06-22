#pragma once
#include "HTTPData.h"

class HTTPSocket
{
public:
	HTTPSocket(int sock);
	HTTPSocket(string host);
	bool isOpened();
	bool isClosed();
	void receive(HTTPData* data);
	void send(HTTPData* data);
	virtual ~HTTPSocket();
};


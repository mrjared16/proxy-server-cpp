#pragma once

#define HTTP_PORT 80
#define BUFFER_SIZE 1000
using namespace std;
class HTTPData;
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


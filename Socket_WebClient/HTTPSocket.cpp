#include "stdafx.h"
#include "HTTPSocket.h"
#include "Connection.h"


HTTPSocket::HTTPSocket(int socket)
{
	this->socket = socket;
	this->open = (socket < 0) ? false : true;
	this->close = (socket < 0) ? true : false;
}


HTTPSocket::HTTPSocket(string hostname)
{
	this->open = false;
	this->close = true;

	struct hostent* hent;
	if ((hent = gethostbyname(hostname.c_str())) == NULL)
	{
		cout << "Can't get IP\n";
		return;
	}

	sockaddr_in* web_address= new sockaddr_in;
	web_address->sin_family = AF_INET;
	memcpy(&web_address->sin_addr, hent->h_addr, hent->h_length);
	web_address->sin_port = htons(HTTP_PORT);

	if (connect(this->socket, (sockaddr*)web_address, sizeof(*web_address)) >= 0)
	{
		this->open = true;
		this->close = false;
	}
	delete web_address;
	delete hent;
}

bool HTTPSocket::isOpened()
{
	return this->open;
}

bool HTTPSocket::isClosed()
{
	return this->close;
}


HTTPSocket::~HTTPSocket()
{
}



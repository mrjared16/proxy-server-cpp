#include "stdafx.h"
#include "HTTPSocket.h"
#include <vector>
#include "HTTPData.h"

HTTPSocket::HTTPSocket(int socket)
{
	this->socket = (socket == INVALID_SOCKET) ? -1 : socket;
	this->is_open = (socket == INVALID_SOCKET) ? false : true;
	this->is_close = (socket == INVALID_SOCKET) ? true : false;
}


HTTPSocket::HTTPSocket(string hostname)
{
	this->socket = -1;
	this->is_open = false;
	this->is_close = true;

	struct hostent* hent;
	if ((hent = gethostbyname(hostname.c_str())) == NULL)
	{
		cout << "Can't get IP\n";
		return;
	}

	sockaddr_in* web_address = new sockaddr_in;
	web_address->sin_family = AF_INET;
	memcpy(&web_address->sin_addr, hent->h_addr, hent->h_length);
	web_address->sin_port = htons(HTTP_PORT);
	// int c = ::connect(this->socket, (sockaddr*)web_address, sizeof(*web_address));
	if (::connect(this->socket, (sockaddr*)web_address, sizeof(*web_address)) == SOCKET_ERROR)
	{
		cout << "Could not connect to host\n";
	}
	else {
		this->is_open = true;
		this->is_close = false;
	}

	delete web_address;
	//free(hent);
}

bool HTTPSocket::isOpened()
{
	return this->is_open;
}

bool HTTPSocket::isClosed()
{
	return this->is_close;
}

// receive and parse to data
bool HTTPSocket::Receive(HTTPData * data)
{
	int body_length = -1;
	string header_first_line, headers;
	vector<char> header_buffer;
	char buffer[BUFFER_SIZE];

	int res;
	// check \r\n\r\n
	bool is_header_end = false;
	while (!is_header_end)
	{
		res = ::recv(this->socket, buffer, BUFFER_SIZE, 0);
		if (res <= 0)
		{
			cout << "Error when send data\n";
			this->is_close = true;
			return false;
		}
		header_buffer.insert(header_buffer.end(), buffer, buffer + res);
		memset(buffer, 0, res);
		string tmp(header_buffer.data());


		// get first line
		if (header_first_line.empty())
		{
			int end_first_line = tmp.find("\r\n");
			if (end_first_line != string::npos)
			{
				header_first_line = tmp.substr(0, end_first_line);
				header_buffer.erase(header_buffer.begin(), header_buffer.begin() + end_first_line + 2);
				tmp = string(header_buffer.data());
			}
		}

		// get body length
		if (body_length == -1)
		{

			int start_content_length = tmp.find("Content-Length");
			if (start_content_length != std::string::npos)
			{
				start_content_length += 16;
				int end_content_length = tmp.find("\r\n", start_content_length);
				if (end_content_length != std::string::npos)
				{
					string length = tmp.substr(start_content_length, end_content_length - start_content_length);
					body_length = atoi(length.c_str());
				}
			}
		}

		// get headers and end loop
		int end_of_header = tmp.find("\r\n\r\n");
		if (end_of_header != string::npos)
		{
			is_header_end = true;
			// have 1 \r\n => +2
			headers = tmp.substr(0, end_of_header + 2);
			header_buffer.erase(header_buffer.begin(), header_buffer.begin() + end_of_header + 4);
			if (body_length == -1)
			{
				body_length = 0;
			}
		}
	}
	// remain data
	vector<char> body(header_buffer);
	int len = body.size();
	while (len < body_length)
	{
		res = ::recv(this->socket, buffer, body_length - len, 0);


		if (res < 0)
		{
			cout << "Error when send data\n";
			this->is_close = true;
			return false;
		}

		body.insert(body.end(), buffer, buffer + res);
		memset(buffer, 0, res);

		len += res;
	}

	data->init(header_first_line, headers, body, body_length);
	return true;
}


bool HTTPSocket::Send(HTTPData * data)
{
	string header = data->getFirstLine() + "\r\n" + data->getHeaders() + "\r\n";
	if (::send(this->socket, header.c_str(), header.length(), 0) < 0)
	{
		cout << "Error when send data\n";
		this->is_close = true;
		return false;
	}

	int body_length = data->getBodyLength();
	int sent = 0, res = -1;

	if (body_length <= 0)
		return false;

	char* body = data->getBody().data();
	while (sent < body_length)
	{
		res = ::send(this->socket, body + sent, body_length - sent, 0);
		if (res < 0)
		{
			cout << "Error when send data\n";
			this->is_close = true;
			return false;
		}
		sent += res;
	}
	return true;
}

HTTPSocket::~HTTPSocket()
{
	::closesocket(this->socket);
}



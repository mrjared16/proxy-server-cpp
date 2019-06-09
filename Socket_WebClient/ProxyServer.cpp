#include "stdafx.h"
#include "ProxyServer.h"




ProxyServer::ProxyServer(int port):
	connection_manager(this)
{
	// INIT
	WSADATA init;
	if (WSAStartup(0x202, &init) != 0) {
		cout << "Error WSAStarup" << endl;
		return;
	}


	// init socket address
	sockaddr_in proxy_address;
	proxy_address.sin_family = AF_INET;
	proxy_address.sin_addr.s_addr = INADDR_ANY;
	proxy_address.sin_port = htons(port);

	// init socket
	this->proxy_server = socket(AF_INET, SOCK_STREAM, 0);
	if (this->proxy_server == INVALID_SOCKET) {
		cout << "Error socket" << endl;
		return;
	}

	// bind sokcet
	int c = ::bind(this->proxy_server, (sockaddr*)& proxy_address, sizeof(proxy_address));
}

ProxyServer::~ProxyServer()
{
	// this->proxy_server.Close();
	closesocket(this->proxy_server);
}

void ProxyServer::run()
{
	this->connection_manager.startListenning();
}

SOCKET ProxyServer::getProxyServerSocket()
{
	return this->proxy_server;
}

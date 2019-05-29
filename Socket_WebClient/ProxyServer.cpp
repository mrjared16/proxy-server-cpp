#include "stdafx.h"
#include "ProxyServer.h"




ProxyServer::ProxyServer(int port):
	connection_manager(this)
{
	this->proxy_server.Create(port);
	this->proxy_server.Listen();
}

ProxyServer::~ProxyServer()
{
	this->proxy_server.Close();
}

void ProxyServer::run()
{
	this->connection_manager.startListenning();
}

CSocket* ProxyServer::getProxyServerSocket()
{
	return &this->proxy_server;
}

#include "stdafx.h"
#include "ConnectionManager.h"
#include "ProxyServer.h"
#include "Connection.h"

ConnectionManager::ConnectionManager(ProxyServer* proxy)
{
	this->parent = proxy;
}

ConnectionManager::~ConnectionManager()
{
	for (auto v : this->online_connections)
		delete v;
}

ProxyServer* ConnectionManager::getProxyServer()
{
	return this->parent;
}

void ConnectionManager::startListenning()
{
	thread t(&ConnectionManager::listenConnection, this);
}

void ConnectionManager::listenConnection()
{
	Connection *connection = new Connection(this);
	// this->online_connections.push_back(connection);
	thread t(&ConnectionManager::listenConnection, this);
	connection->startConnecting();
	delete connection;
}



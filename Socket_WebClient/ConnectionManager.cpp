#include "stdafx.h"
#include "ConnectionManager.h"
#include "ProxyServer.h"
#include "Connection.h"
#include <mutex>

ConnectionManager::ConnectionManager(ProxyServer* proxy)
{
	this->parent = proxy;
}

ConnectionManager::~ConnectionManager()
{
	for (int i = 0; i < online_connections.size(); i++)
	{
		online_connections[i]->join();
	}
	for (int i = 0; i < online_connections.size(); i++)
	{
		delete online_connections[i];
	}
}

ProxyServer* ConnectionManager::getProxyServer()
{
	return this->parent;
}

void ConnectionManager::startListenning()
{
	// start listen
	if (::listen(this->getProxyServer()->getProxyServerSocket(), 5) < 0) {
		cout << "Error listen" << endl;
		return;
	}

	// start thread to listen socket
	thread *t = new thread(&ConnectionManager::listenConnection, this);

	online_connections.push_back(t);
}

void ConnectionManager::listenConnection()
{
	Connection *connection = new Connection(this);
	thread *t = new thread(&ConnectionManager::listenConnection, this);
	online_connections.push_back(t);
	connection->start();
	delete connection;
}



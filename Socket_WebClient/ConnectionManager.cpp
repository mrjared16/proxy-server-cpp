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
	this->proxy_server_socket = this->parent->getProxyServerSocket()->Detach();
	thread *t = new thread(&ConnectionManager::listenConnection, this);
	//t->detach();
	online_connections.push_back(t);
}

void ConnectionManager::listenConnection()
{
	this->parent->getProxyServerSocket()->Attach(this->proxy_server_socket);
	Connection *connection = new Connection(this);
	// this->online_connections.push_back(connection);
	this->proxy_server_socket = this->parent->getProxyServerSocket()->Detach();
	thread *t = new thread(&ConnectionManager::listenConnection, this);
	//t->detach();
	online_connections.push_back(t);
	connection->startConnecting();
	delete connection;
}



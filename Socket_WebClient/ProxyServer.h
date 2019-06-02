#pragma once
#include "afxsock.h"
#include <thread>
#include "ConnectionManager.h"

class ProxyServer
{
public:
	ProxyServer(int port);
	~ProxyServer();
	void run();
	SOCKET getProxyServerSocket();
private:
	ConnectionManager connection_manager;
	SOCKET proxy_server;
};


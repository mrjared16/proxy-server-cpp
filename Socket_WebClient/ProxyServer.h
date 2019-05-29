#pragma once
#include <afxsock.h>
#include <thread>
#include "ConnectionManager.h"

class ProxyServer
{
public:
	ProxyServer(int port);
	~ProxyServer();
	void run();
	CSocket* getProxyServerSocket();
private:
	ConnectionManager connection_manager;
	CSocket proxy_server;
};


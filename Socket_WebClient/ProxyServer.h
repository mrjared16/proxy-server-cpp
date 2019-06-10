#pragma once
#include "afxsock.h"
#include <thread>
#include "ConnectionManager.h"
#include "CacheManager.h"

class ProxyServer
{
public:
	ProxyServer(int port);
	~ProxyServer();

	int getProxyServerSocket();
	CacheManager* getCacheManager();

	void run();
private:
	ConnectionManager connection_manager;
	CacheManager cache_manager;

	int proxy_server;
};


#pragma once

#include <string>
#include <afxsock.h>
#include <thread>
#include <vector>

#define HTTP_PORT 80

using namespace std;

class ProxyServer;
class Connection;

class ConnectionManager 
{
public:
	ConnectionManager(ProxyServer *proxy);
	~ConnectionManager();
	ProxyServer* getProxyServer();
	void startListenning();
private:
	vector<Connection*> online_connections;
	ProxyServer* parent;

	void listenConnection();
};



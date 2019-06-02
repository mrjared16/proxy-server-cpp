#pragma once
#include <string>
#include <afxsock.h>
#include <vector>

#define BUFFER_SIZE 1000
#define TIMEOUT 6
#define HTTP_PORT 80

using namespace std;
class ConnectionManager;

class Connection
{
public:
	Connection(ConnectionManager*);
	void startConnecting();
	~Connection();
private:
	string getRequestFromClient();

	string getStandardizeHTTPRequest();

	string getHostnameFromRequest();

	sockaddr_in* getWebserverAddress();
	void requestProcessing();
	bool isSupport();

	bool sendRequestToWebServer();
	bool transferResponseToClient();

	bool sendDeniedResponse();
private:
	SOCKET client_proxy, proxy_web;
	string url;
	string hostname;
	string request;

private:
	static const vector<string> support_method;
	static const vector<string> support_protocol;
	static const vector<string> support_version;
};


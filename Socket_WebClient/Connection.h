#pragma once
#include <string>
#include <afxsock.h>
#include <vector>

#define BUFFER_SIZE 1000
#define TIMEOUT 6
#define HTTP_PORT 80

class ConnectionManager;
class BlackList;
class CacheManager;


using namespace std;
class Connection
{
public:
	Connection(ConnectionManager*);
	void start();
	~Connection();

private:
	// string getHostnameFromRequest();

	void getRequestFromClient();
	void getURLFromRequest();
	void URLProcessing();
	
	string getStandardizeHTTPRequest();


	sockaddr_in* getWebserverAddress();

	void requestProcessing();
	bool isSupport();

	bool sendRequestToWebServer();
	bool transferResponseToClient();

	bool sendDeniedResponse();

private:
	CacheManager* cache_manager;
	BlackList* blacklist;
	SOCKET client_proxy, proxy_web;

	string request;

	string url;
	
	string protocol;
	string hostname;
	string page;
private:
	static const vector<string> support_method;
	static const vector<string> support_protocol;
	static const vector<string> support_version;
};


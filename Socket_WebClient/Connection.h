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

	bool getRequestFromClient();
	void getURLFromRequestHeader();
	void URLProcessing();
	
	string getStandardizeHTTPRequestHeader();


	sockaddr_in* getWebserverAddress();

	void requestHeaderProcessing();
	bool isSupport();

	bool sendRequestToWebServer();
	bool transferResponseToClient();

	bool sendDeniedResponse();
	
	// header
	void standardizeHeader();
private:
	CacheManager* cache_manager;
	BlackList* blacklist;
	int client_proxy, proxy_web;

	vector<char> request;
	// string request;
	string request_header;

	string url;
	
	string protocol;
	string hostname;
	string page;
private:
	static const vector<string> support_method;
	static const vector<string> support_protocol;
	static const vector<string> support_version;
};


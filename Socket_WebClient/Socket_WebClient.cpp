
// Socket_WebClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Socket_WebClient.h"
#include "ProxyServer.h"
#include <thread>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map> /* cache */

#include "afxsock.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace std;

// const
#define PORT 80
#define PROXY_PORT 8888
#define SIZE 1024
#define TIMEOUT 6 


// GLOBAL FUNCTION
CWinApp theApp;
int thread_counter = 0;
map<string, vector<char>> myCache;

// PROTOTYPE

DWORD WINAPI  acceptClientRequest(LPVOID arg);
DWORD WINAPI  runMsg(LPVOID arg);

// get
bool getHostName(string query, string& host);

char* get_ip(char* host);
wchar_t* convertCharArrayToLPCWSTR(const char* charArray);

string getURL(string query, string protocol);
string standardizeRequest(string query);

// check
bool isServerNameInBlacklist(string server_name);
vector<char> getResponseFromCache(string url);

// cache
void saveToCache(string url, vector<char> response);
void getWebPageAndSendToClient(CSocket* client_proxy, string host, string request);


// void
string getRequest(CSocket* client_proxy);

void LoadBlackList(vector<string>& arr);
bool sendDeniedResponse(CSocket* client_proxy);




// cache

vector<char> getResponseFromCache(string url) {
	return myCache[url];
}

void saveToCache(string url, vector<char> response) {
	myCache[url] = response;
}

// other function
void LoadBlackList(vector<string>& arr)
{
	fstream file;
	file.open("blacklist.conf", ios::in);
	if (file.is_open()) {
		while (!file.eof()) {
			string temp;
			getline(file, temp);
			if (temp.back() == '\n') {
				temp.pop_back();
			}
			arr.push_back(temp);
		}
	}
}

// transfer data
bool sendDeniedResponse(CSocket * client_proxy)
{
	string ResForbidden = "HTTP/1.0 403 Forbidden\r\nCache-Control: no-cache\r\nConnection: close\r\n";
	return (client_proxy->Send(ResForbidden.c_str(), ResForbidden.length(), 0) >= 0);
}

string getRequest(CSocket * client_proxy)
{
	try
	{
		string result;
		char buffer_rec[SIZE + 1] = { 0 };
		int receive_response = 0;

		
		if ((receive_response = client_proxy->Receive(buffer_rec, SIZE, 0)) > 0)
		{
			cout << "Receive from browser: receive_response = " << receive_response << " bytes\n";
			result += buffer_rec;
			//memset(buffer_rec, 0, receive_response);
		}
		else if (receive_response < 0)
		{
			cout << "Cant receive from browser\n";
			return "";
		}
		cout << "\nRequest: \n\t" << result << "\n";
		return result;
	}
	catch (...)
	{
		cout << "Having some error (getRequest) \n";
		return "";
	}
}

void getWebPageAndSendToClient(CSocket * client_proxy, string host, string request)
{
	try {
		int receive_response = 0, send_response = 0;


		// 1. CHECK IN CACHE

		//Kiem tra xem da co response tu server gui ve lan nao chua? Neu co thi lay response ra tu cache va proxy gui ve cho client
		//con neu khong thi proxy gui request len server de lay response ve
		vector<char> cache = getResponseFromCache(getURL(request, "http://"));

		int cache_size = cache.size();

		if (cache_size > 0) {
			char* buffer = new char[cache_size];
			copy(cache.begin(), cache.end(), buffer);

			int sent = 0;
			while (sent < cache_size)
			{
				send_response = client_proxy->Send(buffer + sent, cache_size, 0);
				if (send_response <= 0)
					break;
				sent += send_response;
			}

			delete[] buffer;
			return;
		}

		// 2. SEND REQUEST


			// 2.1. GET IP AND CONNECT TO WEB SERVER

				// 2.1.1. GET IP

		char* tmp = strdup(host.c_str());
		char* ip = get_ip(tmp);
		delete tmp;
		if (ip == NULL)
		{
			return;
		}


		// 2.1.2. CONNECT TO WEB SERVER

		CSocket proxy_web;
		proxy_web.Create();

		cout << "connecting to " << ip << " ...\n";
		if (proxy_web.Connect(convertCharArrayToLPCWSTR(ip), PORT) < 0)
		{
			cout << "Could not connect";
			proxy_web.Close();
			return;
		}

		cout << "Connected!\n";
		delete[] ip;

		// 2.2. SEND REQUEST TO WEB SERVER

		// replace url by page
		string request_header = standardizeRequest(request);

		int sent = 0;
		int request_header_length = request_header.length();

		//Send the request_header to the server
		while (sent < request_header_length)
		{
			cout << "Sending request_header...\n";
			send_response = proxy_web.Send(request_header.substr(sent).c_str(), request_header_length - sent, 0);

			cout << "Send to web server: send_response = " << send_response << " bytes\n";

			// send error
			if (send_response < 0) {
				cout << "Can't send request";
				proxy_web.Close();
				return;
			}

			// send done
			if (send_response == 0)
			{
				break;
			}

			sent += send_response;
		}


		// 3. RECEIVE AND SEND DATA TO CLIENT

		cout << "Start receive data...\n";

		char receive_buffer[SIZE + 1];
		memset(receive_buffer, 0, sizeof(receive_buffer));

		// timeout when receive from web server
		timeval timeout_webserver = { TIMEOUT, 0 };

		// timeout when send to client
		timeval timeout_client = { TIMEOUT, 0 };

		fd_set in_set;

		while (true) {
			FD_ZERO(&in_set);
			FD_SET(proxy_web, &in_set);

			// set timeout
			int cnt = select(proxy_web + 1, &in_set, NULL, NULL, &timeout_webserver);

			if (FD_ISSET(proxy_web, &in_set))
			{
				// receive from web server
				receive_response = proxy_web.Receive(receive_buffer, SIZE, 0);

				// error when receive or done
				if (receive_response <= 0)
					break;

				cout << "Received from web server: receive_response = " << receive_response << " bytes\n";

				cout << "Sending to client...: \n\t" << receive_buffer << "\n";

				// send to client

				// TODO: create thread for send and insert to cache
				int send_res = client_proxy->Send(receive_buffer, receive_response, 0);

				// send error: client side
				if (send_res <= 0)
					break;

				// add to cache
				cache.insert(cache.end(), receive_buffer, receive_buffer + receive_response);

				// clear buffer
				memset(receive_buffer, 0, receive_response);
			}
			else {
				saveToCache(getURL(request, "http://"), cache);
				break;

			}
		}
		proxy_web.Close();
	}
	catch (...)
	{
		cout << "Having some error (getWebPageAndSendToClient)\n";
	}
}


// thread
DWORD WINAPI  acceptClientRequest(LPVOID arg) // TODO: convert to CWinThread/ std::thread
{
	try {
		SOCKET* socket = (SOCKET*)arg;
		
		CSocket proxy_server;
		if (proxy_server.Attach(*socket) == 0)
		{
			return 0;
		}

		CSocket client_proxy;
		proxy_server.Accept(client_proxy);
		
		cout << "**********************************************************";
		cout << "\nGot connect!\n";
		cout << "**********************************************************\n";
		
		thread_counter++;
		
		if ((*socket = proxy_server.Detach()) == 0)
		{
			return 0;
		}

		// start new thread to accept
		DWORD next_ThreadID;
		HANDLE h = CreateThread(NULL, 0, acceptClientRequest, socket, 0, &next_ThreadID);
		
		
		// this thread use to connect to client
		string request_from_client = getRequest(&client_proxy);
		
		string hostname;
		
		// get hostname and check if method/ protocol is support
		// and check if hostname is in blacklist
		if (!getHostName(request_from_client, hostname) || isServerNameInBlacklist(hostname))
		{
			sendDeniedResponse(&client_proxy);
		}

		else
		{
			// nhan du lieu tu web gui cho client
			getWebPageAndSendToClient(&client_proxy, hostname, request_from_client);
		}

		// done, close connect
		client_proxy.Close();
		
		thread_counter--;
		return 0;
	}
	catch (...)
	{
		cout << "Having some error (accept)!\n";
		return 0;
	}
}

DWORD __stdcall runMsg(LPVOID arg)
{
	while (1)
	{
		Sleep(3000);
		cout << "So thread dang ket noi: " << thread_counter << "\n";
	}
	return 0;
}


/* Ref code:http://coding.debuntu.org/c-linux-socket-programming-tcp-simple-http-client */

int main(int argc, char* argv[])
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(NULL);

	if (hModule != NULL)
	{
		// initialize MFC and print and error on failure
		if (!AfxWinInit(hModule, NULL, ::GetCommandLine(), 0))
		{
			// TODO: change error code to suit your needs
			_tprintf(_T("Fatal Error: MFC initialization failed\n"));
			nRetCode = 1;
		}
		else
		{
			// TODO: code your application's behavior here.
			/* Ref code:http://coding.debuntu.org/c-linux-socket-programming-tcp-simple-http-client */
			//CSocket proxy_server;
			AfxSocketInit(NULL);
			//proxy_server.Create(8888);
			//proxy_server.Listen();
			//SOCKET* socket = new SOCKET();
			//*socket = proxy_server.Detach();
			//DWORD threadID;
			//// acceptClientRequest(&proxy_server);
			//HANDLE h1 = CreateThread(NULL, 0, acceptClientRequest, socket, 0, &threadID);
			//HANDLE h2 = CreateThread(NULL, 0, runMsg, NULL, 0, &threadID);
			//system("pause");
			//myCache.clear();
			//proxy_server.Close();
			ProxyServer proxy_server(PROXY_PORT);
			proxy_server.run();
			system("pause");
		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	system("pause");
	return nRetCode;
}






char* get_ip(char* host)
{
	struct hostent* hent;
	int iplen = 15; //XXX.XXX.XXX.XXX
	char* ip = new char[iplen + 1];
	memset(ip, 0, iplen + 1);
	if ((hent = gethostbyname(host)) == NULL)
	{
		cout << "Can't get IP";
		return NULL;
	}
	if (inet_ntop(AF_INET, (void*)hent->h_addr_list[0], ip, iplen) == NULL)
	{
		cout << "Can't resolve host";
		return NULL;
	}
	return ip;
}

wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}



bool isServerNameInBlacklist(string hostname) {
	vector<string> blacklist;

	LoadBlackList(blacklist);

	if (blacklist.size() == 0) {
		cout << "Khong load duoc file blacklist" << endl;
		return false;
	}

	for (int i = 0, size = blacklist.size(); i < size; i++) {
		// found in blacklist
		if (blacklist[i].find(hostname) != -1) {
			return true;
		}
	}

	return false;
}

bool getHostName(string query, string & host)
{
	//GET http://www.abc.com/abc/ HTTP1.0 => GET /abc HTTP/1.0
	//method|| protocal+host+page|| protocol
	string protocol = "http://";
	string url, page, method;
	if ((query.find("GET") == -1 && query.find("POST") == -1) || query.find(protocol) == -1)
	{
		cout << "Chuong trinh khong ho tro method hay protocol nay!\n";
		return false;
	}
	url = getURL(query, protocol);
	//url = http://www.abc.com/abc/
	host = url.substr(protocol.length(), url.find('/', protocol.length() + 1) - protocol.length());
	// cout << "url: " << url << endl;
	// cout << "host: " << host << endl;
	// cout << "page: " << page << endl;
	return true;
}

string getURL(string query, string protocol) {
	return query.substr(query.find(protocol), query.find("HTTP/") - 2 - query.find(protocol));
}

string standardizeRequest(string query) {
	string url, host, page;
	url = getURL(query, "http://");
	getHostName(query, host);
	page = url.substr(url.find(host) + host.length(), url.length() - url.find(host) - host.length() + 1);
	// replace url by page
	query.replace(query.find("http://"), url.length(), page);
	// cout << "standardize request: " << query << endl;
	return query;
}

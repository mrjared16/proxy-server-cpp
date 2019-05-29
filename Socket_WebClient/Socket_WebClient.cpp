
// Socket_WebClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Socket_WebClient.h"
#include <thread>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <map> /* cache */

/* Khai bao thu vien */
#include "afxsock.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/* Declare function */
bool sendDeniedResponse(CSocket* client_proxy);
char* get_ip(char* host);
wchar_t* convertCharArrayToLPCWSTR(const char* charArray);

#define PORT 80
#define SIZE 1000000
// The one and only application object


CWinApp theApp;

using namespace std;


DWORD WINAPI  acceptClientRequest(LPVOID arg);
DWORD WINAPI  runMsg(LPVOID arg);

int thread_counter = 0;
map<string, vector<char>> myCache;

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

bool isServerNameInBlacklist(string server_name) {
	vector<string> blacklist;
	LoadBlackList(blacklist);
	if (blacklist.size() == 0) {
		cout << "Khong load duoc file blacklist" << endl;
	}
	for (int i = 0; i < blacklist.size(); i++) {
		if (blacklist[i].find(server_name) != -1) {
			return true;
		}
	}
	return false;
}

string getURL(string query, string protocol) {
	return query.substr(query.find(protocol), query.find("HTTP/") - 2 - query.find(protocol));
}

void storeIntoCache(string url, vector<char> response) {
	myCache[url] = response;
}

vector<char> getResponseFromCache(string url) {
	return myCache[url];
}

string getRequest(CSocket* client_proxy)
{
	try
	{
		string result;
		char buffer_rec[SIZE + 1] = { 0 };
		int tmp_res = 0;
		if ((tmp_res = client_proxy->Receive(buffer_rec, SIZE, 0)) > 0)
		{
			cout << "Receive from browser: tmp_res = " << tmp_res << " bytes\n";
			if (buffer_rec) {
				result += buffer_rec;
			}
			memset(buffer_rec, 0, tmp_res);
		}
		else
		{
			cout << "Cant receive from browser\n";
			return "";
		}
		cout << "\nRequest: \n\t" << result << "\n";
		return result;
	}
	catch (int a)
	{
		cout << "Having some error (getRequest) \n";
	}
}

bool getHostName(string query, string& host)
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

void getWebPageAndSendToClient(CSocket* client_proxy, string host, string query)
{

	try {
		//Kiem tra xem da co response tu server gui ve lan nao chua? Neu co thi lay response ra tu cache va proxy gui ve cho client
		//con neu khong thi proxy gui request len server de lay response ve
		vector<char> response = getResponseFromCache(getURL(query, "http://"));
		if (response.size() != 0) {
			char *buffer = new char[response.size()];
			copy(response.begin(), response.end(), buffer);
			client_proxy->Send(buffer, response.size(), 0);
			return;
		}
		//Buoc 3: Ket noi toi Server
		char* tmp = strdup(host.c_str());
		char* ip = get_ip(tmp);
		delete tmp;
		//fprintf(stderr, "IP is %s\n", ip);
		if (ip == NULL)
		{
			return;
		}

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
		//fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", query);
		string request = standardizeRequest(query);
		//Send the query to the server
		int sent = 0;
		int tmp_res = 0;
		while (sent < request.length())
		{
			cout << "Sending query...\n";
			tmp_res = proxy_web.Send(request.substr(sent).c_str(), request.length() - sent, 0);
			cout << "Send to web server: tmp_res = " << tmp_res << " bytes\n";
			if (tmp_res == -1 || tmp_res == 0) {
				cout << "Can't send query";
				proxy_web.Close();
				return;
			}
			sent += tmp_res;
		}
		cout << "Start receive data...\n";
		//now it is time to receive the page
		char buffer_rec[SIZE + 1];
		memset(buffer_rec, 0, sizeof(buffer_rec));
		/*int htmlstart = 0;
		char * htmlcontent;*/
		timeval timeout = { 6, 0 };
		fd_set in_set;
		while (true) {
			FD_ZERO(&in_set);
			FD_SET(proxy_web, &in_set);
			int cnt = select(proxy_web + 1, &in_set, NULL, NULL, &timeout);
			if (FD_ISSET(proxy_web, &in_set))
			{
				tmp_res = proxy_web.Receive(buffer_rec, SIZE, 0);
				response.insert(response.end(), buffer_rec, buffer_rec + tmp_res);
				cout << "Received from web server: tmp_res = " << tmp_res << " bytes\n";
				if (buffer_rec) {
					//fprintf(stdout, buffer_rec);
					client_proxy->Send(buffer_rec, tmp_res, 0);
					cout << "Sending to client...: \n\t" << buffer_rec << "\n";
					memset(buffer_rec, 0, tmp_res);
				}
			}
			else {
				storeIntoCache(getURL(query, "http://"), response);
				proxy_web.Close();
				return;
			}
		}
		if (tmp_res <= 0)
		{
			cout << "Error receiving data\n";
		}
		//free(get);
		//free(ip);
		proxy_web.Close();
	}
	catch (int a)
	{
		cout << "Having some error (getWebPageAndSendToClient)\n";
	}
}

//Ref: http://stackoverflow.com/questions/19715144/how-to-convert-char-to-lpcwstr
wchar_t* convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
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
			CSocket proxy_server;
			AfxSocketInit(NULL);
			proxy_server.Create(8888);
			proxy_server.Listen();
			SOCKET* socket = new SOCKET();
			*socket = proxy_server.Detach();
			DWORD threadID;
			// acceptClientRequest(&proxy_server);
			HANDLE h1 = CreateThread(NULL, 0, acceptClientRequest, socket, 0, &threadID);
			HANDLE h2 = CreateThread(NULL, 0, runMsg, NULL, 0, &threadID);
			system("pause");
			myCache.clear();
			proxy_server.Close();
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

/*
Get ip from domain name
*/
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

bool sendDeniedResponse(CSocket * client_proxy)
{
	string ResForbidden = "HTTP/1.0 403 Forbidden\r\n\Cache-Control: no-cache\r\n\Connection: close\r\n";
	return (client_proxy->Send(ResForbidden.c_str(), ResForbidden.length(), 0) >= 0);
}

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
		// nhan request tu client
		DWORD next_ThreadID;
		// while (thread_counter > 1)
		// {
			// Sleep(5000);
			// cout << "Wait for other request...\n";
		// }
		HANDLE h = CreateThread(NULL, 0, acceptClientRequest, socket, 0, &next_ThreadID);
		string query = getRequest(&client_proxy);
		string host_name;
		if (!getHostName(query, host_name) || isServerNameInBlacklist(host_name))
		{
			sendDeniedResponse(&client_proxy);
		}
		// nhan du lieu tu web gui cho client
		else
		{
			getWebPageAndSendToClient(&client_proxy, host_name, query);
		}
		client_proxy.Close();
		thread_counter--;
		return 0;
	}
	catch (int a)
	{
		cout << "Having some error (accept)!\n";
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



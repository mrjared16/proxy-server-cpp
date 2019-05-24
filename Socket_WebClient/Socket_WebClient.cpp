
// Socket_WebClient.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Socket_WebClient.h"

/* Khai bao thu vien */
#include "afxsock.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/* Declare function */
void sendDeniedResponse(CSocket* client_proxy);
int create_tcp_socket();
char* get_ip(char* host);
wchar_t* convertCharArrayToLPCWSTR(const char* charArray);
char* build_get_query(char* host, char* page);
int main(int argc, char* argv[]);
void usage();

#define HOST "www.fit.hcmus.edu.vn"
#define PAGE "/"
#define PORT 80
#define USERAGENT "HTMLGET 1.0"
#define SIZE 10000
// The one and only application object


CWinApp theApp;

using namespace std;


void getWebPageAndSendToClient(CSocket* client_proxy, string host, string query)
{
	CSocket proxy_web;
	proxy_web.Create();

	//Buoc 3: Ket noi toi Server
	char* tmp = strdup(host.c_str());
	char* ip = get_ip(tmp);
	//fprintf(stderr, "IP is %s\n", ip);
	if (ip == NULL)
	{
		return;
	}
	cout << "connecting to " << ip << " ...\n";
	if (proxy_web.Connect(convertCharArrayToLPCWSTR(ip), PORT) < 0)
	{
		cout << "Could not connect";
		return;
	}
	cout << "Connected!\n";
	//fprintf(stderr, "Query is:\n<<START>>\n%s<<END>>\n", query);

	//Send the query to the server
	int sent = 0;
	int tmp_res = 0;
	while (sent < query.length())
	{
		cout << "Sending query...\n";
		tmp_res = proxy_web.Send(query.substr(sent).c_str(), query.length() - sent, 0);
		if (tmp_res == -1) {
			cout << "Can't send query";
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
	if ((tmp_res = proxy_web.Receive(buffer_rec, SIZE, 0)) > 0)
	{
		if (buffer_rec) {
			//fprintf(stdout, buffer_rec);
			client_proxy->Send(buffer_rec, tmp_res, 0);
			cout << "Sending to client...: \n\t" << buffer_rec << "\n";
			memset(buffer_rec, 0, tmp_res);
		}
	}
	if (tmp_res < 0)
	{
		cout << "Error receiving data";
	}
	//free(get);
	//free(ip);
	proxy_web.Close();
}


string getRequest(CSocket* client_proxy)
{
	string result;
	char buffer_rec[SIZE + 1] = { 0 };
	int tmp_res = 0;
	if/*while*/ ((tmp_res = client_proxy->Receive(buffer_rec, SIZE, 0)) > 0)
	{
		if (tmp_res > 0) {
			result += buffer_rec;
		}
		memset(buffer_rec, 0, tmp_res);
	}
	cout << "Request: \n\t" << result << "\n";
	return result;
}

bool getHostName(string& query, string& host)
{
	//GET http://www.abc.com/abc/ HTTP1.0 => GET /abc HTTP/1.0
	//method|| protocal+host+page|| protocol
	string protocol = "http://";
	string url, page, method;
	if (query.find("GET") == -1 || query.find(protocol) == -1)
	{
		cout << "Chuong trinh khong ho tro method hay protocol nay!\n";
		return false;
	}
	url = query.substr(query.find(protocol), query.find("HTTP/") - 2 - query.find(protocol));
	//url = http://www.abc.com/abc/
	host = url.substr(protocol.length(), url.find('/', protocol.length() + 1) - protocol.length());
	page = url.substr(url.find(host) + host.length(), url.length() - url.find(host) - host.length() + 1);
	cout << "url: " << url << endl;
	cout << "host: " << host << endl;
	cout << "page: " << page << endl;
	// replace url by page
	query.replace(query.find(protocol), url.length(), page);
	cout << "standardize request: " << query << endl;
	return true;
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
			CSocket proxy_server, * client_proxy;
			AfxSocketInit(NULL);
			proxy_server.Create(8888);
			proxy_server.Listen();
			while (1)
			{
				client_proxy = new CSocket();
				proxy_server.Accept(*client_proxy);
				// nhan request tu client
				string query = getRequest(client_proxy);
				//const char* query_ch = query.c_str();
				// lay hostname tu request
				string host_name;
				if (!getHostName(query, host_name))
				{

					sendDeniedResponse(client_proxy);
				}
				// nhan du lieu tu web gui cho client
				else
				{
					getWebPageAndSendToClient(client_proxy, host_name, query);
				}
				client_proxy->Close();
				delete client_proxy;
			}

		}
	}
	else
	{
		// TODO: change error code to suit your needs
		_tprintf(_T("Fatal Error: GetModuleHandle failed\n"));
		nRetCode = 1;
	}

	return nRetCode;
}

void usage()
{
	//fprintf(stderr, "USAGE: htmlget host [page]\n\
		\thost: the website hostname. ex: coding.debuntu.org\n\
		\tpage: the page to retrieve. ex: index.html, default: /\n");
}

/*
Get ip from domain name
*/
char* get_ip(char* host)
{
	struct hostent* hent;
	int iplen = 15; //XXX.XXX.XXX.XXX
	char* ip = (char*)malloc(iplen + 1);
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

void sendDeniedResponse(CSocket* client_proxy)
{
	string ResForbidden = "HTTP/1.0 403 Forbidden\r\n\Cache-Control: no-cache\r\n\Connection: close\r\n";
	client_proxy->Send(ResForbidden.c_str(), ResForbidden.length(), 0);
}
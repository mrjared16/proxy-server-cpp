#include "stdafx.h"
#include "Connection.h"
#include "ConnectionManager.h"
#include "ProxyServer.h"

static const vector<string> support_version = { "HTTP1.0", "HTTP1.1" };
static const vector<string> support_method = { "GET", "POST" };
static const vector<string> support_protocol = { "http://" };

Connection::Connection(ConnectionManager* connect_mng)
{
	connect_mng->getProxyServer()->getProxyServerSocket()->Accept(this->client_proxy);
}


void Connection::startConnecting()
{
	this->request = this->getRequestFromClient();
	if (!isSupport())
		return;
	// prepare for sending and receiving data
	this->proxy_web.Create();

	// get hostname
	this->requestProcessing();

	// send standardize request to web server
	this->sendRequestToWebServer();

	// receive data from web server and send to client
	this->transferResponseToClient();

	// close connect to web server
	this->proxy_web.Close();
}

Connection::~Connection()
{
	this->client_proxy.Close();
}

string Connection::getRequestFromClient()
{
	try
	{
		string result;
		char buffer[BUFFER_SIZE + 1] = { 0 };
		int response_size = 0;
		if ((response_size = client_proxy.Receive(buffer, BUFFER_SIZE, 0)) > 0)
		{
			cout << "Receive from browser: response_size = " << response_size << " bytes\n";
			if (buffer != "") {
				result += buffer;
			}
			memset(buffer, 0, response_size);
		}
		else
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
	}
}

string Connection::getStandardizeHTTPRequest()
{
	string result = request;
	int pos_protocol = request.find("http://");
	int len_protocol = 7;
	int len_host = hostname.length();
	result.replace(pos_protocol, len_protocol + len_host, "");
	return result;
}

string Connection::getHostnameFromRequest()
{
	string result;
	int pos_protocol = request.find("http://");
	int len_protocol = 7;
	int pos_hostname = pos_protocol + len_protocol;
	int pos_page = request.find('/', pos_hostname + 1);
	result = request.substr(pos_hostname, pos_page - pos_hostname - 1);
	return result;
}


// GET http://www.abc.com/abc/ HTTP1.0
// => GET /abc HTTP/1.0
// <method> <protocol><hostname><page> protocol
void Connection::requestProcessing()
{
	string protocol = "http://";

	//url = getURLFromReqeust();
	//url = http://www.abc.com/abc/
	hostname = getHostnameFromRequest();
}

bool Connection::isSupport()
{
	bool check[3] = { false, false, false };
	vector<string> support[3] = { support_method, support_protocol, support_version };
	for (int i = 0; i < 3; i++)
	{
		for (string str : support[i])
		{
			if (request.find(str) != -1)
			{
				check[i] = true;
				break;
			}
		}
		if (!check[i])
			return false;
	}

	return true;
}

bool Connection::sendRequestToWebServer()
{
	char* c_char_hostname = strdup(hostname.c_str());
	char* ip = getIPFromHost(c_char_hostname);
	free(c_char_hostname);
	string standard_request = getStandardizeHTTPRequest();
	cout << "connecting to " << ip << " ...\n";
	wchar_t* ip_wstr = convertCharArrayToLPCWSTR(ip);
	if (proxy_web.Connect(ip_wstr, HTTP_PORT) < 0)
	{
		cout << "Could not connect";
		delete[] ip_wstr;
		return false;
	}
	int sent = 0;
	int tmp_res = 0;
	while (sent < request.length())
	{
		cout << "Sending query...\n";
		tmp_res = proxy_web.Send(request.substr(sent).c_str(), request.length() - sent, 0);
		cout << "Send to web server: tmp_res = " << tmp_res << " bytes\n";
		if (tmp_res == -1 || tmp_res == 0) {
			cout << "Can't send query";
			break;
		}
		sent += tmp_res;
	}

	if (ip != NULL)
		delete[] ip;
	delete[] ip_wstr;
	return true;
}

bool Connection::transferResponseToClient()
{
	char buffer_rec[BUFFER_SIZE + 1];
	memset(buffer_rec, 0, sizeof(buffer_rec));
	int response_size;
	timeval timeout = { 6, 0 };
	fd_set in_set;
	while (true) {
		FD_ZERO(&in_set);
		FD_SET(proxy_web, &in_set);
		int cnt = select(proxy_web + 1, &in_set, NULL, NULL, &timeout);
		if (FD_ISSET(proxy_web, &in_set))
		{
			response_size = proxy_web.Receive(buffer_rec, BUFFER_SIZE, 0);
			cout << "Received from web server: response_size = " << response_size << " bytes\n";
			if (buffer_rec) {
				//fprintf(stdout, buffer_rec);
				client_proxy.Send(buffer_rec, response_size, 0);
				cout << "Sending to client...: \n\t" << buffer_rec << "\n";
				memset(buffer_rec, 0, response_size);
			}
		}
	}
	if (response_size <= 0)
	{
		cout << "Error receiving data\n";
	}
}

char* Connection::getIPFromHost(char* hostname)
{
	struct hostent* hent;
	if ((hent = gethostbyname(hostname)) == NULL)
	{
		cout << "Can't get IP";
		return NULL;
	}

	int iplen = 15; //XXX.XXX.XXX.XXX
	char* ip = new char[iplen + 1];
	memset(ip, 0, iplen + 1);

	if (inet_ntop(AF_INET, (void*)hent->h_addr_list[0], ip, iplen) == NULL)
	{
		cout << "Can't resolve host";
		delete[] ip;
		return NULL;
	}
	return ip;
}


//Ref: http://stackoverflow.com/questions/19715144/how-to-convert-char-to-lpcwstr
wchar_t* Connection::convertCharArrayToLPCWSTR(const char* charArray)
{
	wchar_t* wString = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, 0, charArray, -1, wString, 4096);
	return wString;
}




bool Connection::sendDeniedResponse(CSocket * client_proxy)
{
	string ResForbidden = "HTTP/1.0 403 Forbidden\r\n\Cache-Control: no-cache\r\n\Connection: close\r\n";
	return (client_proxy->Send(ResForbidden.c_str(), ResForbidden.length(), 0) >= 0);
}
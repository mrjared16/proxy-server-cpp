#include "stdafx.h"
#include "Connection.h"
#include "ConnectionManager.h"
#include "ProxyServer.h"

const vector<string> Connection::support_version = { "HTTP/1.0", "HTTP/1.1" };
const vector<string> Connection::support_method = { "GET", "POST" };
const vector<string> Connection::support_protocol = { "http://" };

Connection::Connection(ConnectionManager* connect_mng)
{
	ProxyServer* proxy_server = connect_mng->getProxyServer();
	CSocket* proxy_server_socket = proxy_server->getProxyServerSocket();
	proxy_server_socket->Accept(this->client_proxy);
	//connect_mng->getProxyServer()->getProxyServerSocket()->Accept(this->client_proxy);
}

void Connection::startConnecting()
{
	try {
		this->request = this->getRequestFromClient();

		if (this->request != "" && !isSupport())
		{
			cout << "\nNot support!\n";
			this->sendDeniedResponse();
			return;
		}


		// OPEN CONNECT TO WEB SERVER 
		this->proxy_web.Create();



		// Get hostname
		this->requestProcessing();


		// Send standardize request to web server
		// Fail to send
		if (!this->sendRequestToWebServer())
		{
			this->proxy_web.Close();
			return;
		}

		// Receive data from web server and send to client
		if (!this->transferResponseToClient())
		{
			this->proxy_web.Close();
			return;
		}



		// CLOSE CONNECT TO WEB SERVER
		this->proxy_web.Close();
	}
	catch (...)
	{
		cout << "Error..\n";
	}
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
		return "";
	}
}

string Connection::getStandardizeHTTPRequest()
{
	string result = request;
	int pos_protocol = request.find("http://");
	int len_protocol = 7;
	int len_host = hostname.length();
	result.replace(pos_protocol, len_protocol + len_host, "");
	cout << "send to web server: " << result << endl;
	return result;
}

string Connection::getHostnameFromRequest()
{
	string result;
	int pos_protocol = request.find("http://");
	int len_protocol = 7;
	int pos_hostname = pos_protocol + len_protocol;
	int pos_page = request.find('/', pos_hostname + 1);
	result = request.substr(pos_hostname, pos_page - pos_hostname);
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
	this->hostname = getHostnameFromRequest();
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
	// 2. SEND REQUEST

		// 2.1. GET IP AND CONNECT TO WEB SERVER

			// 2.1.1. GET IP

	// Get host ip
	char* c_char_hostname = strdup(hostname.c_str());
	char* ip = getIPFromHost(c_char_hostname);
	free(c_char_hostname);

	if (ip == NULL)
		return false;

	// 2.1.2. CONNECT TO WEB SERVER


// connect to web server
	cout << "\nConnecting to " << ip << " ...\n";
	wchar_t* ip_wstr = convertCharArrayToLPCWSTR(ip);

	if (proxy_web.Connect(ip_wstr, HTTP_PORT) < 0)
	{
		cout << "Could not connect to host ip!!!\n";
		delete[] ip_wstr;
		return false;
	}

	cout << "Connected!\n";
	delete[] ip;
	delete[] ip_wstr;


	// 2.2. SEND REQUEST TO WEB SERVER

	int send_response = 0;

	// replace url by page
	string request_header = getStandardizeHTTPRequest();

	int sent = 0;
	int request_header_length = request_header.length();

	// Send the request_header to the server
	while (sent < request_header_length)
	{
		cout << "Sending request_header...\n";
		send_response = proxy_web.Send(request_header.substr(sent).c_str(), request_header_length - sent, 0);

		cout << "Send to web server: send_response = " << send_response << " bytes\n";

		// send error
		if (send_response < 0) {
			cout << "Error when send request to web server\n";
			return false;
		}

		// send done
		if (send_response == 0)
		{
			break;
		}

		sent += send_response;
	}
	return true;
}

bool Connection::transferResponseToClient()
{
	int receive_response = 0, send_response = 0;

	// 3. RECEIVE AND SEND DATA TO CLIENT

	cout << "Start receive data from web server...\n";

	char receive_buffer[BUFFER_SIZE + 1] = { 0 };

	// timeout when receive from web server
	timeval timeout_webserver = { TIMEOUT, 0 };

	// timeout when send to client
	timeval timeout_client = { TIMEOUT, 0 };

	fd_set in_set;

	while (true) {
		FD_ZERO(&in_set);
		FD_SET(this->proxy_web, &in_set);

		// set timeout
		int cnt = select(this->proxy_web + 1, &in_set, NULL, NULL, &timeout_webserver);

		if (FD_ISSET(this->proxy_web, &in_set))
		{
			// receive from web server
			receive_response = this->proxy_web.Receive(receive_buffer, BUFFER_SIZE, 0);

			// error when receive or done
			if (receive_response < 0)
			{
				cout << "Error when receive from web server\n";
				return false;
			}
			else if (receive_response == 0) {
				break;
			}

			cout << "Received from web server: receive_response = " << receive_response << " bytes\n";

			cout << "Forwarding to client...: \n\t" << receive_buffer << "\n";

			// send to client

			// TODO: create thread for send and insert to cache
			send_response = this->client_proxy.Send(receive_buffer, receive_response, 0);

			// send error: client side
			if (send_response < 0)
			{
				cout << "Error when send to client \n";
				return false;
			}
			else if (send_response == 0) {
				break;
			}
			// add to cache
			//cache.insert(cache.end(), receive_buffer, receive_buffer + receive_response);

			// clear buffer
			memset(receive_buffer, 0, receive_response);
		}
		else {
			//saveToCache(getURL(request, "http://"), cache);
			break;

		}
	}
	return true;
}

char* Connection::getIPFromHost(char* hostname)
{
	struct hostent* hent;
	if ((hent = gethostbyname(hostname)) == NULL)
	{
		cout << "Can't get IP\n";
		return NULL;
	}

	int iplen = 15; //XXX.XXX.XXX.XXX
	char* ip = new char[iplen + 1];
	memset(ip, 0, iplen + 1);

	if (inet_ntop(AF_INET, (void*)hent->h_addr_list[0], ip, iplen) == NULL)
	{
		cout << "Can't resolve host\n";
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




bool Connection::sendDeniedResponse()
{
	string ResForbidden = "HTTP/1.0 403 Forbidden\r\nCache-Control: no-cache\r\nConnection: close\r\n";
	return (this->client_proxy.Send(ResForbidden.c_str(), ResForbidden.length(), 0) >= 0);
}
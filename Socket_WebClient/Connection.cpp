#include "stdafx.h"
#include "Connection.h"
#include "ConnectionManager.h"
#include "ProxyServer.h"
#include "BlackList.h"
#include "CacheManager.h"


const vector<string> Connection::support_version = { "HTTP/1.0", "HTTP/1.1" };
const vector<string> Connection::support_method = { "GET", "POST" };
const vector<string> Connection::support_protocol = { "http://" };

Connection::Connection(ConnectionManager* connect_mng)
{
	// ProxyServer* proxy_server = connect_mng->getProxyServer();
	// CSocket* proxy_server_socket = proxy_server->getProxyServerSocket();
	// proxy_server_socket->Accept(this->client_proxy);
	//connect_mng->getProxyServer()->getProxyServerSocket()->Accept(this->client_proxy);

	sockaddr_in client_address;
	int size = sizeof(client_address);

	this->client_proxy = ::accept(connect_mng->getProxyServer()->getProxyServerSocket(), (sockaddr*)& client_address, &size);

	this->blacklist = new BlackList();

	this->cache_manager = connect_mng->getProxyServer()->getCacheManager();
}

Connection::~Connection()
{
	::closesocket(this->client_proxy);
	delete this->blacklist;
}

// ********************************************//

void Connection::start()
{
	if (this->client_proxy == INVALID_SOCKET) {
		cout << "Error accept" << endl;
		return;
	}
	try {
		this->getRequestFromClient();

		// check if support (HTTP/1.0, 1.1; http://; GET, POST)
		if (this->request != "" && !isSupport())
		{
			cout << "\nNot support!\n";
			this->sendDeniedResponse();
			return;
		}

		// Get url, hostname, protocol, page
		this->requestProcessing();


		if (this->blacklist != NULL && blacklist->isExist(this->hostname))
		{
			cout << "\nHost in blacklist!\n";
			this->sendDeniedResponse();
			return;
		}


		if (this->cache_manager != NULL && this->cache_manager->isExist(this->url))
		{
			cout << "Already in cache! Just forward to client...\n";
			vector<char>* cache_data = this->cache_manager->getResponse(this->url);

			int sent = 0;
			int send_response = 0;
			int len = cache_data->size();

			while (sent < len)
			{
				send_response = ::send(this->client_proxy, cache_data->data() + sent, len - sent, 0);

				cout << "Send to client: send_response = " << send_response << " bytes\n";
				cout << cache_data->data() << endl;
				// send error
				if (send_response < 0) {
					cout << "Error when send request to web server\n";
					break;
				}

				// send done
				if (send_response == 0)
				{
					break;
				}

				sent += send_response;
			}

			if (cache_data != NULL)
			{
				delete cache_data;
			}

			return;
		}


		// OPEN CONNECT TO WEB SERVER 
		this->proxy_web = ::socket(AF_INET, SOCK_STREAM, 0);
		if (this->proxy_web == INVALID_SOCKET) {
			cout << "Error socket proxy web" << endl;
			return;
		}

		// Send standardize request to web server
		// Fail to send
		if (!this->sendRequestToWebServer())
		{
			::closesocket(this->proxy_web);
			return;
		}

		// Receive data from web server and send to client
		if (!this->transferResponseToClient())
		{
			::closesocket(this->proxy_web);
			return;
		}


		// CLOSE CONNECT TO WEB SERVER
		::closesocket(this->proxy_web);
	}
	catch (...)
	{
		cout << "Error..\n";
	}
}

void Connection::getRequestFromClient()
{
	try
	{
		char buffer[BUFFER_SIZE + 1] = { 0 };
		int response_size = 0;
		if ((response_size = ::recv(this->client_proxy, buffer, BUFFER_SIZE, 0)) > 0)
		{
			cout << "Receive from browser: response_size = " << response_size << " bytes\n";
			if (buffer != "") {
				this->request += buffer;
			}
			memset(buffer, 0, response_size);
		}
		else
		{
			cout << "Cant receive from browser\n";
			this->request = "";
			return;
		}
		cout << "\nRequest: \n\t" << this->request << "\n";
		return;
	}
	catch (...)
	{
		cout << "Having some error (getRequest) \n";
		this->request = "";
		return;
	}
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

void Connection::requestProcessing()
{
	this->getURLFromRequest();
	this->URLProcessing();
}

bool Connection::sendRequestToWebServer()
{
	// 2. SEND REQUEST

		// 2.1. GET WEB SERVER ADDRESS

	sockaddr_in* web_address = this->getWebserverAddress();

	if (web_address == NULL)
	{
		return false;
	}

	// 2.2. CONNECT TO WEB SERVER

	if (connect(this->proxy_web, (sockaddr*)web_address, sizeof(*web_address)) < 0)
	{
		cout << "Could not connect to host!!!\n";
		return false;
	}

	delete web_address;
	cout << "Connected!\n";

	// 2.3. SEND REQUEST TO WEB SERVER

	int send_response = 0;

	// replace url by page
	string request_header = getStandardizeHTTPRequest();

	int sent = 0;
	int request_header_length = request_header.length();

	// Send the request_header to the server
	cout << "Sending request_header...\n";
	while (sent < request_header_length)
	{
		send_response = ::send(this->proxy_web, request_header.substr(sent).c_str(), request_header_length - sent, 0);

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
	bool cache = true;
	bool first = true;
	while (true) {
		FD_ZERO(&in_set);
		FD_SET(this->proxy_web, &in_set);

		// set timeout
		int cnt = select(this->proxy_web + 1, &in_set, NULL, NULL, &timeout_webserver);

		if (FD_ISSET(this->proxy_web, &in_set))
		{
			// receive from web server
			receive_response = ::recv(this->proxy_web, receive_buffer, BUFFER_SIZE, 0);

			// error when receive or done
			if (receive_response < 0)
			{
				this->cache_manager->clear(this->url);
				cout << "Error when receive from web server\n";
				return false;
			}
			else if (receive_response == 0) {
				break;
			}
			else {
				if (first)
				{
					string header(receive_buffer, 20);
					if (header.find("200 OK") == -1)
					{
						cache = false;
					}
					first = false;
				}
				if (cache)
					this->cache_manager->append(this->url, receive_buffer, receive_response);
			}
			cout << "Received from web server: receive_response = " << receive_response << " bytes\n";

			//cout << "Forwarding to client...: \n\t" << receive_buffer << "\n";

			// send to client

			// TODO: create thread for send and insert to cache
			send_response = ::send(this->client_proxy, receive_buffer, receive_response, 0);

			// send error: client side
			if (send_response < 0)
			{
				this->cache_manager->clear(this->url);
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

bool Connection::sendDeniedResponse()
{
	string ResForbidden = "HTTP/1.0 403 Forbidden\r\nCache-Control: no-cache\r\nConnection: close\r\n";
	return (::send(this->client_proxy, ResForbidden.c_str(), ResForbidden.length(), 0) >= 0);
}





// *******************************************//

// GET http://www.abc.com/abc/ HTTP1.0
// => GET /abc HTTP/1.0
// <method> <protocol><hostname><page> protocol

void Connection::getURLFromRequest()
{
	for (string protocol : this->support_protocol)
	{
		int start = request.find(protocol);
		if (start != -1)
		{
			int end = request.find(' ', start) - 1;
			this->url = request.substr(start, end - start + 1);
			break;
		}
	}
}

// GET http://www.abc.com/abc/ HTTP1.0
// => GET /abc HTTP/1.0
// <method> <protocol><hostname><page> protocol

void Connection::URLProcessing()
{
	if (this->url == "")
		return;
	int start_protocol = 0;
	int start_hostname = this->url.find('/') + 2;
	int start_page = this->url.find('/', start_hostname);

	this->protocol = this->url.substr(start_protocol, start_hostname - start_protocol);
	this->hostname = this->url.substr(start_hostname, start_page - start_hostname);
	this->page = this->url.substr(start_page);
}


// GET http://www.abc.com/abc/ HTTP1.0
// => GET /abc HTTP/1.0
// <method> <protocol><hostname><page> protocol

string Connection::getStandardizeHTTPRequest()
{
	/*string result = request;
	int pos_protocol = request.find("http://");
	int len_protocol = 7;

	int len_host = hostname.length();
	result.replace(pos_protocol, len_protocol + len_host, "");
	cout << "send to web server: " << result << endl;*/
	string result = request;
	result.replace(result.find(url), url.length(), page);
	return result;
}

sockaddr_in* Connection::getWebserverAddress()
{
	struct hostent* hent;
	if ((hent = gethostbyname(hostname.c_str())) == NULL)
	{
		cout << "Can't get IP\n";
		return NULL;
	}

	sockaddr_in* result = new sockaddr_in;
	result->sin_family = AF_INET;
	memcpy(&result->sin_addr, hent->h_addr, hent->h_length);
	result->sin_port = htons(HTTP_PORT);

	return result;
}


// => GET /abc HTTP/1.0
// <method> <protocol><hostname><page> protocol
// string Connection::getHostnameFromRequest()
// {
// 	string result;
// 	int pos_protocol = request.find("http://");
// 	int len_protocol = 7;
// 
// 	int pos_hostname = pos_protocol + len_protocol;
// 	int pos_page = request.find('/', pos_hostname + 1);
// 
// 	result = request.substr(pos_hostname, pos_page - pos_hostname);
// 
// 	return result;
// }
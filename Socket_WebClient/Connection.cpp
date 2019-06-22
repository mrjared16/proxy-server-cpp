#include "stdafx.h"
#include "Connection.h"
#include "ConnectionManager.h"
#include "ProxyServer.h"
#include "BlackList.h"
#include "CacheManager.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"



const vector<string> Connection::support_version = { "HTTP/1.0", "HTTP/1.1" };
const vector<string> Connection::support_method = { "GET", "POST" };
const vector<string> Connection::support_protocol = { "http://" };

Connection::Connection(ConnectionManager* connect_mng)
{
	this->cache_manager = connect_mng->getProxyServer()->getCacheManager();
	this->blacklist = new BlackList();

	//	this->client_proxy = ::accept(connect_mng->getProxyServer()->getProxyServerSocket(), (sockaddr*)& client_address, &size);

	sockaddr_in client_address;
	int size = sizeof(client_address);
	int sock_client = ::accept(connect_mng->getProxyServer()->getProxyServerSocket(), (sockaddr*)& client_address, &size);

	this->client_proxy = new HTTPSocket(sock_client);
	this->proxy_web = NULL;
	this->request = NULL;
	this->response = NULL;
}

Connection::~Connection()
{
	//::closesocket(this->client_proxy);
	delete this->client_proxy;
	delete this->blacklist;
}

// ********************************************//

void Connection::start()
{
	if (!this->client_proxy->isOpened())
	{
		cout << "Error when open socket to recv from client\n";
		return;
	}

	this->request = new HTTPRequest();
	this->client_proxy->Receive(request);

	// not support
	if (!this->isSupport((HTTPRequest*)this->request))
	{
		cout << "Not support this request \n";
		this->response = this->getDeniedResponse();
		this->client_proxy->Send(this->response);

		delete this->request;
		delete this->response;

		return;
	}

	// in blacklist
	string host = ((HTTPRequest*)request)->getHostname();
	if (this->blacklist != NULL && this->blacklist->isExist(host))
	{
		cout << "Host in blacklist\n";
		this->response = this->getDeniedResponse();
		this->client_proxy->Send(this->response);

		delete this->request;
		delete this->response;
		return;
	}

	// in cache
	string url = ((HTTPRequest*)request)->getURL();
	if (this->cache_manager != NULL && this->cache_manager->isExist(url))
	{
		// get cache
		this->cache_manager->getResponse(url, (HTTPResponse * &)this->response);
		this->client_proxy->Send(this->response);

		delete this->request;
	}
	else {
		// get response from web server
		this->proxy_web = new HTTPSocket(host);
		if (!this->proxy_web->isOpened())
		{
			cout << "Error connect to web server!\n";

			this->response = this->getCantResolveHostResponse();
			this->client_proxy->Send(this->response);

			delete this->request;
			delete this->response;
			delete this->proxy_web;

			return;
		}
		this->proxy_web->Send(this->request);

		this->response = new HTTPResponse();
		this->proxy_web->Receive(this->response);
		// slower than old version
		// only sent to client when received all data
		this->client_proxy->Send(this->response);
		this->cache_manager->insert(url, (HTTPResponse*)this->response);

		delete this->request;
		delete this->response;
		delete this->proxy_web;
	}

}
// if (this->client_proxy == INVALID_SOCKET) {
// 	cout << "Error accept" << endl;
// 	return;
// }
// 	try {
// 		// error or not support
// 		if (!this->getRequestFromClient())
// 		{
// 			return;
// 		}
// 
// 		// Get url, hostname, protocol, page
// 		this->requestHeaderProcessing();
// 
// 
// 		if (this->blacklist != NULL && blacklist->isExist(this->hostname))
// 		{
// 			cout << "\nHost in blacklist!\n";
// 			this->sendDeniedResponse();
// 			return;
// 		}
// 
// 
// 		if (this->cache_manager != NULL && this->cache_manager->isExist(this->url))
// 		{
// 			cout << "Already in cache! Just forward to client...\n";
// 			vector<char>* cache_data = this->cache_manager->getResponse(this->url);
// 
// 			int sent = 0;
// 			int send_response = 0;
// 			int len = cache_data->size();
// 
// 			while (sent < len)
// 			{
// 				send_response = ::send(this->client_proxy, cache_data->data() + sent, len - sent, 0);
// 
// 				cout << "Send to client: send_response = " << send_response << " bytes\n";
// 				cout << cache_data->data() << endl;
// 				// send error
// 				if (send_response < 0) {
// 					cout << "Error when send request to web server\n";
// 					break;
// 				}
// 
// 				// send done
// 				if (send_response == 0)
// 				{
// 					break;
// 				}
// 
// 				sent += send_response;
// 			}
// 
// 			if (cache_data != NULL)
// 			{
// 				delete cache_data;
// 			}
// 
// 			return;
// 		}
// 
// 
// 		// OPEN CONNECT TO WEB SERVER 
// 		this->proxy_web = ::socket(AF_INET, SOCK_STREAM, 0);
// 		if (this->proxy_web == INVALID_SOCKET) {
// 			cout << "Error socket proxy web" << endl;
// 			return;
// 		}
// 
// 		// Send standardize request to web server
// 		// Fail to send
// 		if (!this->sendRequestToWebServer())
// 		{
// 			::closesocket(this->proxy_web);
// 			return;
// 		}
// 
// 		// Receive data from web server and send to client
// 		if (!this->transferResponseToClient())
// 		{
// 			::closesocket(this->proxy_web);
// 			return;
// 		}
// 
// 
// 		// CLOSE CONNECT TO WEB SERVER
// 		::closesocket(this->proxy_web);
// 	}
// 	catch (...)
// 	{
// 		cout << "Error..\n";
// 	}
//}
//
//bool Connection::getRequestFromClient()
//{
//	try
//	{
//		char buffer[BUFFER_SIZE + 1] = { 0 };
//
//		bool first = true;
//		int response_size = 0;
//
//		timeval timeout = { 3, 0 };
//		fd_set in_set;
//		while (true) {
//			FD_ZERO(&in_set);
//			FD_SET(this->client_proxy, &in_set);
//			int cnt = select(this->client_proxy + 1, &in_set, NULL, NULL, &timeout);
//			if (FD_ISSET(this->client_proxy, &in_set))
//			{
//
//				response_size = ::recv(this->client_proxy, buffer, BUFFER_SIZE, 0);
//				if (response_size > 0)
//				{
//					if (first)
//					{
//						int i = 0;
//						while (buffer[i] != '\n')
//						{
//							i++;
//						}
//
//						this->request_header = string(buffer, i + 1);
//
//						if (!this->isSupport())
//						{
//							cout << this->request_header << " not support!\n";
//							this->sendDeniedResponse();
//
//							return false;
//						}
//
//						first = false;
//					}
//
//					cout << "Receive from browser: response_size = " << response_size << " bytes\n";
//
//					if (buffer != "") {
//						this->request.insert(this->request.end(), buffer, buffer + response_size);
//					}
//
//					memset(buffer, 0, response_size);
//
//				}
//				else {
//					break;
//
//				}
//			}
//			else
//			{
//				break;
//			}
//		}
//		if (first)
//		{
//			cout << "Cant receive from browser\n";
//			//this->request_header = "";
//			return false;
//		}
//
//		cout << "\nRequest: \n\t" << this->request.data() << "\n";
//		return true;
//	}
//	catch (...)
//	{
//		cout << "Having some error (getRequest) \n";
//		//this->request_header = "";
//		return false;
//	}
//}
//

bool Connection::isSupport(HTTPRequest* req)
{
	string start_line = req->getStartLine();
//	string request_properties[3] = {req->getMethod(), req->getProtocol(), req->getVersion()};
	bool check[3] = { false, false, false };
	vector<string> support[3] = { support_method, support_protocol, support_version };
	for (int i = 0; i < 3; i++)
	{
		for (string str : support[i])
		{
			if (start_line.find(str) != string::npos)
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

HTTPResponse* Connection::getDeniedResponse()
{
	HTTPResponse* result = new HTTPResponse();
	vector<char> null_vector;
	result->init("HTTP/1.0 403 Forbidden",
		"Cache-Control: no-cache\r\nConnection: close\r\n", null_vector, 0);
	return result;
}
HTTPResponse* Connection::getCantResolveHostResponse()
{
	HTTPResponse* result = new HTTPResponse();
	vector<char> null_vector;
	result->init("HTTP/1.0 403 Forbidden",
		"Cache-Control: no-cache\r\nConnection: close\r\n", null_vector, 0);
	return result;

}

HTTPResponse* Connection::getTimeoutResponse()
{
	HTTPResponse* result = new HTTPResponse();
	vector<char> null_vector;
	result->init("HTTP/1.0 403 Forbidden",
		"Cache-Control: no-cache\r\nConnection: close\r\n", null_vector, 0);
	return result;
}
//
//void Connection::requestHeaderProcessing()
//{
//	this->getURLFromRequestHeader();
//	this->URLProcessing();
//}
//
//bool Connection::sendRequestToWebServer()
//{
//	// 2. SEND REQUEST
//
//		// 2.1. GET WEB SERVER ADDRESS
//
//	sockaddr_in* web_address = this->getWebserverAddress();
//
//	if (web_address == NULL)
//	{
//		return false;
//	}
//
//	// 2.2. CONNECT TO WEB SERVER
//
//	if (connect(this->proxy_web, (sockaddr*)web_address, sizeof(*web_address)) < 0)
//	{
//		cout << "Could not connect to host!!!\n";
//		return false;
//	}
//
//	delete web_address;
//	cout << "Connected!\n";
//
//	// 2.3. SEND REQUEST TO WEB SERVER
//
//	int send_response = 0;
//
//	// replace url by page
//	this->standardizeHeader();
//
//	int sent = 0;
//	int request_header_length = this->request.size();
//
//	// Send the request_header to the server
//	cout << "Sending request_header...\n";
//	while (sent < request_header_length)
//	{
//		send_response = ::send(this->proxy_web, this->request.data() + sent, request_header_length - sent, 0);
//
//		cout << "Send to web server: send_response = " << send_response << " bytes\n";
//
//		// send error
//		if (send_response < 0) {
//			cout << "Error when send request to web server\n";
//			return false;
//		}
//
//		// send done
//		if (send_response == 0)
//		{
//			break;
//		}
//
//		sent += send_response;
//	}
//	return true;
//}
//
//bool Connection::transferResponseToClient()
//{
//	int receive_response = 0, send_response = 0;
//
//	// 3. RECEIVE AND SEND DATA TO CLIENT
//
//	cout << "Start receive data from web server...\n";
//
//	char receive_buffer[BUFFER_SIZE + 1] = { 0 };
//
//	// timeout when receive from web server
//	timeval timeout_webserver = { TIMEOUT, 0 };
//
//	// timeout when send to client
//	timeval timeout_client = { TIMEOUT, 0 };
//
//	fd_set in_set;
//	bool cache = true;
//	bool first = true;
//	while (true) {
//		FD_ZERO(&in_set);
//		FD_SET(this->proxy_web, &in_set);
//
//		// set timeout
//		int cnt = select(this->proxy_web + 1, &in_set, NULL, NULL, &timeout_webserver);
//
//		if (FD_ISSET(this->proxy_web, &in_set))
//		{
//			// receive from web server
//			receive_response = ::recv(this->proxy_web, receive_buffer, BUFFER_SIZE, 0);
//
//			// error when receive or done
//			if (receive_response < 0)
//			{
//				this->cache_manager->clear(this->url);
//				cout << "Error when receive from web server\n";
//				return false;
//			}
//			else if (receive_response == 0) {
//				break;
//			}
//			else {
//				if (first)
//				{
//					string header(receive_buffer, 20);
//					if (header.find("GET 200 OK") == -1)
//					{
//						cache = false;
//					}
//					first = false;
//				}
//				if (cache)
//					this->cache_manager->append(this->url, receive_buffer, receive_response);
//			}
//			cout << "Received from web server: receive_response = " << receive_response << " bytes\n";
//
//			//cout << "Forwarding to client...: \n\t" << receive_buffer << "\n";
//
//			// send to client
//
//			// TODO: create thread for send and insert to cache
//			send_response = ::send(this->client_proxy, receive_buffer, receive_response, 0);
//
//			// send error: client side
//			if (send_response < 0)
//			{
//				this->cache_manager->clear(this->url);
//				cout << "Error when send to client \n";
//				return false;
//			}
//			else if (send_response == 0) {
//				break;
//			}
//
//			// clear buffer
//			memset(receive_buffer, 0, receive_response);
//		}
//		else {
//			break;
//
//		}
//	}
//	return true;
//}
//
//bool Connection::sendDeniedResponse()
//{
//	string ResForbidden = "HTTP/1.0 403 Forbidden\r\nCache-Control: no-cache\r\nConnection: close\r\n";
//	return (::send(this->client_proxy, ResForbidden.c_str(), ResForbidden.length(), 0) >= 0);
//}
//
//// *******************************************//
//
//// GET http://www.abc.com/abc/ HTTP1.0
//// => GET /abc HTTP/1.0
//// <method> <protocol><hostname><page> protocol
//
//void Connection::getURLFromRequestHeader()
//{
//	for (string protocol : this->support_protocol)
//	{
//		int start = this->request_header.find(protocol);
//		if (start != -1)
//		{
//			int end = this->request_header.find(' ', start) - 1;
//			this->url = this->request_header.substr(start, end - start + 1);
//			break;
//		}
//	}
//}
//
//// GET http://www.abc.com/abc/ HTTP1.0
//// => GET /abc HTTP/1.0
//// <method> <protocol><hostname><page> protocol
//
//void Connection::URLProcessing()
//{
//	if (this->url == "")
//		return;
//	int start_protocol = 0;
//	int start_hostname = this->url.find('/') + 2;
//	int start_page = this->url.find('/', start_hostname);
//
//	this->protocol = this->url.substr(start_protocol, start_hostname - start_protocol);
//	this->hostname = this->url.substr(start_hostname, start_page - start_hostname);
//	this->page = this->url.substr(start_page);
//}
//
//
//// GET http://www.abc.com/abc/ HTTP1.0
//// => GET /abc HTTP/1.0
//// <method> <protocol><hostname><page> protocol
//void Connection::standardizeHeader()
//{
//	string standard_header = this->getStandardizeHTTPRequestHeader();
//	const char* tmp = standard_header.c_str();
//	this->request.erase(this->request.begin(), this->request.begin() + this->request_header.length());
//	this->request.insert(this->request.begin(), tmp, tmp + standard_header.length());
//}
//
//string Connection::getStandardizeHTTPRequestHeader()
//{
//	string result = request_header;
//	result.replace(result.find(url), url.length(), page);
//	return result;
//}
//
//sockaddr_in* Connection::getWebserverAddress()
//{
//	struct hostent* hent;
//	if ((hent = gethostbyname(hostname.c_str())) == NULL)
//	{
//		cout << "Can't get IP\n";
//		return NULL;
//	}
//
//	sockaddr_in* result = new sockaddr_in;
//	result->sin_family = AF_INET;
//	memcpy(&result->sin_addr, hent->h_addr, hent->h_length);
//	result->sin_port = htons(HTTP_PORT);
//
//	return result;
//}

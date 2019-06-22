#include "stdafx.h"
#include "HTTPRequest.h"

HTTPRequest::HTTPRequest()
{
}


HTTPRequest::~HTTPRequest()
{
}

//GET http://www.abc.com/abc/ HTTP1.1
//Tu first line (dong dau tien trong du lieu nhan) phan tich va lay ra
//method, protocol, hostname, page va version
void HTTPRequest::handle(string first_line)
{
	int start_method = 0;
	int start_protocol = first_line.find(" ") + 1;
	int start_hostname = first_line.find('/') + 2;
	int start_page = first_line.find('/', start_hostname);
	int start_version = first_line.find(" ", start_page) + 1;

	this->method = first_line.substr(start_method, start_protocol - 1);
	this->protocol = first_line.substr(start_protocol, start_hostname - start_protocol);
	this->hostname = first_line.substr(start_hostname, start_page - start_hostname);
	this->page = first_line.substr(start_page, start_version - start_page - 1);
	this->version = first_line.substr(start_version);
}

//url = http://www.abc.com/abc/
//url = protocol + hostname + page
string HTTPRequest::getURL()
{
	string url = this->protocol + this->hostname + this->page;
	return url;
}

//start line = GET /abc HTTP/1.0
//start line = method + page + version
string HTTPRequest::getFirstLine()
{
	string start_line = this->method + " " + this->page + " " + this->version;
	return start_line;
}

string HTTPRequest::getHostname()
{
	return this->hostname;
}


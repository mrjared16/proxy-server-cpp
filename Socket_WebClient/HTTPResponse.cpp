#include "stdafx.h"
#include "HTTPResponse.h"


HTTPResponse::HTTPResponse()
{
}


HTTPResponse::~HTTPResponse()
{
}

//HTTP/1.1 404 Not Found (first line)
//Tu first line (dong dau tien trong du lieu nhan) phan tich va lay ra
//protocol version, status code va status text
void HTTPResponse::handle(string status_line)
{
	int start_protocol_version = 0;
	int start_status_code = status_line.find(" ") + 1;
	int start_status_text = status_line.find(" ", start_status_code) + 1;

	this->protocol_version = status_line.substr(start_protocol_version, start_status_code - 1);
	this->status_code = status_line.substr(start_status_code, start_status_text - start_status_code - 1);
	this->status_text = status_line.substr(start_status_text);
}

//Chi luu lai response neu headers co code = 200 va khong co no-cache
bool HTTPResponse::isCache()
{
	if (this->status_code != "200") {
		return false;
	}
	else {
		if (this->header.find("no-cache") != string::npos) {
			return false;
		}
		else {
			return true;
		}
	}
}

//status line = HTTP/1.1 404 Not Found
//status line = protocol version + status code + status text
string HTTPResponse::getStatusLine()
{
	string status_line = this->protocol_version + " " + this->status_code + " " + this->status_text;
	return status_line;
}

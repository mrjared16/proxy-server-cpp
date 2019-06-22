#include "stdafx.h"
#include "HTTPData.h"


HTTPData::HTTPData()
{
	this->body_length = -1;
}


HTTPData::~HTTPData()
{
}

string HTTPData::getFirstLine()
{
	return this->first_line;
}

string HTTPData::getHeaders()
{
	return this->headers;
}

vector<char> HTTPData::getBody()
{
	return this->body;
}

int HTTPData::getBodyLength()
{
	return this->body_length;
}

void HTTPData::init(string first_line, string headers, vector<char> body, int body_length)
{
	this->first_line = first_line;
	this->headers = headers;
	this->body = body;
	this->body_length = body_length;
	
	// init status line/ start line
	this->handle();
}

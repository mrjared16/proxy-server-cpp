#pragma once
#include "HTTPData.h"
class HTTPRequest :
	public HTTPData
{
public:
	HTTPRequest();
	virtual ~HTTPRequest();
	void handle(string header_line);
	bool isSuport();
	string getURL();
	string getHost();
private:
	string method;
	string protocol;
	string host;
	string page;
	string version;
};


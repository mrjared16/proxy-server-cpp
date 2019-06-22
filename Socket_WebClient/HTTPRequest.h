#pragma once
#include "HTTPData.h"
class HTTPRequest :
	public HTTPData
{
public:
	HTTPRequest();
	virtual ~HTTPRequest();
	void handle(string first_line);
	string getURL();
	string getFirstLine();
	string getHostname();
private:
	string method;
	string protocol;
	string hostname;
	string page;
	string version;
};


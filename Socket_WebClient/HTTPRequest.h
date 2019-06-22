#pragma once
#include "HTTPData.h"
class HTTPRequest :
	public HTTPData
{
public:
	HTTPRequest();
	virtual ~HTTPRequest();

	string getFirstLine();
	void handle();

	// string getStartLine();

	string getURL();
	string getHostname();
private:
	string method;
	string protocol;
	string hostname;
	string page;
	string version;
};


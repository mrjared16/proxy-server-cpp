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
	string getMethod();
	string getVersion();
	string getProtocol();
private:
	string method;
	string version;
	// url
	string protocol;
	string hostname;
	string page;
};


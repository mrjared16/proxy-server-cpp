#pragma once
#include "HTTPData.h"
class HTTPRequest :
	public HTTPData
{
public:
	HTTPRequest();
	virtual ~HTTPRequest();
	void init(string header_line);
	bool isSuport();
	string getURL();
private:
	string method;
	string protocol;
	string host;
	string page;
	string version;
};


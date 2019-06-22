#pragma once
#include "HTTPData.h"
class HTTPResponse :
	public HTTPData
{
public:
	HTTPResponse();
	virtual ~HTTPResponse();
	void handle(string status_line);
	bool isCache();
	string getFirstLine();
private:
	string protocol_version;
	string status_code;
	string status_text;
};


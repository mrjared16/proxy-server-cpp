#pragma once
#include "HTTPData.h"
class HTTPResponse :
	public HTTPData
{
public:
	HTTPResponse();
	virtual ~HTTPResponse();

	// string getFirstLine();
	void handle();

	bool isCache();
private:
	string protocol_version;
	string status_code;
	string status_text;
};


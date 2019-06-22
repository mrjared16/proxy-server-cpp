#pragma once
#include "HTTPData.h"
class HTTPResponse :
	public HTTPData
{
public:
	HTTPResponse();
	virtual ~HTTPResponse();
	void init(string header_line);
	bool isCache();
private:
	string version;
	string status_code;
};


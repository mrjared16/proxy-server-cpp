#pragma once
#include <map>
#include <string>
#include <vector>
#include "HTTPResponse.h"

using namespace std;
class CacheManager
{
public:
	CacheManager();
	~CacheManager();

	// void insert(const string &url, const vector<char> &response);
	void insert(const string &url, const HTTPResponse *response);
	// void append(const string& url, const char* s, int len);
	void clear(const string& url);
	bool isExist(const string &url);

	HTTPResponse* getResponse(const string &url);

private:
	map<string, HTTPResponse> my_cache;
};


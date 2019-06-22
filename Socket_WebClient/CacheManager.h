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
	// void append(const string& url, const char* s, int len);
	void insert(const string &url, HTTPResponse *response);
	void clear(const string& url);
	bool isExist(const string &url);

	void getResponse(const string &url, HTTPResponse *&response);

private:
	map<string, HTTPResponse> my_cache;
};


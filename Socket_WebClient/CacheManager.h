#pragma once
#include <map>
#include <string>
#include <vector>
using namespace std;
class CacheManager
{
public:
	CacheManager();
	~CacheManager();

	void insert(const string &url, const vector<char> &response);
	void append(const string& url, const char* s, int len);
	void clear(const string& url);
	bool isExist(const string &url);

	vector<char>* getResponse(const string &url);

private:
	map<string, vector<char>> my_cache;
};


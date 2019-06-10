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
	bool isExist(const string &url);

	vector<char>* getResponse(const string &url);

private:
	map<string, vector<char>> my_cache;
};


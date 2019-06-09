#pragma once
#include <map>
#include <string>
#include <vector>
using namespace std;
class Cache
{
public:
	Cache();
	~Cache();
	void insert(string url, vector<char>);
	bool isExist(string url);
	vector<char> getResponse(string url);
private:
	map<string, vector<char>> myCache;
};


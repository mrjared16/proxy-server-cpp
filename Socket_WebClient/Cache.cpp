#include "stdafx.h"
#include "Cache.h"


Cache::Cache()
{
}


Cache::~Cache()
{
	this->myCache.clear();
}

void Cache::insert(string url, vector<char> response)
{
	this->myCache[url] = response;
}

bool Cache::isExist(string url)
{
	if (this->myCache[url].size() == 0)
		return false;
	else
		return true;
}

vector<char> Cache::getResponse(string url)
{
	return this->myCache[url];
}

#include "stdafx.h"
#include "CacheManager.h"


CacheManager::CacheManager()
{
}


CacheManager::~CacheManager()
{
	this->my_cache.clear();
}

void CacheManager::insert(const string &url, const vector<char> &response)
{
	this->my_cache[url] = response;
}

bool CacheManager::isExist(const string &url)
{
	if (this->my_cache[url].size() == 0)
		return false;
	else
		return true;
}

vector<char>* CacheManager::getResponse(const string &url)
{
	return new vector<char>(this->my_cache[url]);
}

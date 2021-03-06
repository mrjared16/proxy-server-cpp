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

void CacheManager::append(const string& url, const char* s, int len)
{
	this->my_cache[url].insert(this->my_cache[url].end(), s, s + len);
	//this->my_cache[url] += s;
}

void CacheManager::clear(const string& url)
{
	this->my_cache.erase(url);
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



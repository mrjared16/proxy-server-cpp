#pragma once
#include <vector>
#include <string>
#include <fstream>
using namespace std;
class BlackList
{
public:
	BlackList();
	~BlackList();
	bool isExist(string host);
private:
	vector<string> blacklist_descriptor;
};


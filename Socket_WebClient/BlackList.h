#pragma once
#include <vector>
#include <string>
#include <fstream>


#define CONFIG_FILE "blacklist.conf"


using namespace std;



class BlackList
{
public:
	BlackList();
	~BlackList();

	bool isExist(const string &host);

private:
	vector<string> blacklist_descriptor;
};


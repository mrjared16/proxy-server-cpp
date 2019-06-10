#include "stdafx.h"
#include "BlackList.h"


BlackList::BlackList()
{
	fstream file;
	file.open(CONFIG_FILE, ios::in);
	if (file.is_open()) {
		while (!file.eof()) {
			string tmp;
			getline(file, tmp);
			if (tmp.back() == '\n') {
				tmp.pop_back();
			}
			this->blacklist_descriptor.push_back(tmp);
		}
	}
}


BlackList::~BlackList()
{
}

bool BlackList::isExist(const string& host)
{
	for (int i = 0; i < this->blacklist_descriptor.size(); i++) {
		if (this->blacklist_descriptor[i] == host) {
			return true;
		}
	}
	return false;
}

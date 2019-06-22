#pragma once
#include <vector>
using namespace std;
class HTTPData
{
public:
	HTTPData();
	virtual ~HTTPData();
	string getHeader();
	string getBody();
	int getBodyLength();
	virtual string getHeaderLine();
protected:
	int body_length;
	string header_line;
	string header;
	vector<char> body;
};


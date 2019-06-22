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
	void init(string header_line, string header, vector<char> body, int body_length);
	virtual void handle(string header_line);
protected:
	int body_length;
	string header_line;
	string header;
	vector<char> body;
};


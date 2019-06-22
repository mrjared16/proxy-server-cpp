#pragma once
#include <vector>
using namespace std;
class HTTPData
{
public:
	HTTPData();
	virtual ~HTTPData();
	string getHeaders();
	vector<char> getBody();
	int getBodyLength();
	void init(string first_line, string headers, vector<char> body, int body_length);
	virtual void handle(string first_line) = 0;
protected:
	string first_line;
	string headers;
	vector<char> body;
	int body_length;
};


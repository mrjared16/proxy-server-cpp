#pragma once
#include <vector>
using namespace std;
class HTTPData
{
public:
	HTTPData();
	virtual ~HTTPData();

	virtual string getFirstLine();
	virtual void handle() = 0;

	string getHeaders();
	vector<char> getBody();

	int getBodyLength();

	void init(const string &first_line, const string &headers, const vector<char> &body, const int &body_length);
protected:
	// status line / start line
	string first_line;
	string headers;
	vector<char> body;
	int body_length;
};


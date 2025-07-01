#pragma once
#include <string>

class Error
{
public:
	Error();
	~Error();

	void err(std::string msg);
};
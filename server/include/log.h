#pragma once
#include <iostream>
#include <string>

class Logger
{
public:
	void error(std::string msg);
	void info(std::string msg);
};
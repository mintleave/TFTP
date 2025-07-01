#include "Error.h"
#include <iostream>

Error::Error()
{
	
}
Error::~Error()
{

}

void Error::err(std::string msg)
{
	std::cout << "error: " << msg << std::endl;
}
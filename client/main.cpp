#include <iostream>
#include <string>
#include "Client.h"

int main()
{

	std::string ip = "127.0.0.1";
	TFTPClient client(ip);
	client.run();
	return 0;
}
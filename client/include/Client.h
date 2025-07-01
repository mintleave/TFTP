#pragma once
#include "Macros.h"
#include "Error.h"

#include <iostream>

class TFTPClient
{
public:
	TFTPClient(const std::string& serverIP);
	~TFTPClient();

	// 下载文件
	void execDownload();
	// 上传文件
	void execUpload();

	void clearScream();
	void waitForInput();
	void showMenu();

	void run();
private:
	// 服务器端口
	static const int PORT = 7788;
	// 数据包大小
	static const int BUFFER_SIZE = 516;

	// 客户端套接字
	SOCKET_TYPE cfd;
	// 服务器地址信息
	sockaddr_in server_addr;

	Error error;
};
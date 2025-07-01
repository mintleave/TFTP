#pragma once
#include "Macros.h"
#include "Error.h"

#include <iostream>

class TFTPClient
{
public:
	TFTPClient(const std::string& serverIP);
	~TFTPClient();

	// �����ļ�
	void execDownload();
	// �ϴ��ļ�
	void execUpload();

	void clearScream();
	void waitForInput();
	void showMenu();

	void run();
private:
	// �������˿�
	static const int PORT = 7788;
	// ���ݰ���С
	static const int BUFFER_SIZE = 516;

	// �ͻ����׽���
	SOCKET_TYPE cfd;
	// ��������ַ��Ϣ
	sockaddr_in server_addr;

	Error error;
};
#include "Client.h"

TFTPClient::TFTPClient(const std::string& serverIP)
{
	INIT_SOCKET();

	// ����ַ��Ϣ�ṹ��
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(PORT);
	server_addr.sin_addr.s_addr = inet_addr(serverIP.c_str());

	// ����UDP�׽���
	cfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (cfd < 0)
	{
		error.err("sock error");
		return;
	}
}

TFTPClient::~TFTPClient()
{
	CLOSE_SOCKET(cfd);
	CLEANUP_SOCKET();
}


void TFTPClient::execDownload()
{
	std::string filename;
	std::cout << "�����ļ���: ";
	std::getline(std::cin, filename);

	// ��װ��������
	char buf[BUFFER_SIZE] = "";
	int size = sprintf(buf, "%c%c%s%c%s%c", 0, 1, filename.c_str(), 0, "octet", 0);

	// ���������������
	if (sendto(cfd, buf, BUFFER_SIZE, 0, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		error.err("sendto error");
		return;
	}

	size_t recv_len;
	unsigned short num = 1;
	socklen_t addr_len = sizeof(server_addr);

	int flag = 0;
	int fd;

	while (true)
	{
		memset(buf, 0, sizeof(buf));

		recv_len = recvfrom(cfd, buf, BUFFER_SIZE, 0, (sockaddr*)&server_addr, &addr_len);
		if (recv_len < 0)
		{
			error.err("recvfrom error");
			return;
		}

		if (buf[1] == 3)
		{
			if (flag == 0)
			{
				fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);
				if (fd < 0)
				{
					error.err("open error");
					return;
				}
				flag = 1;
			}

			if (htons(num) == *(unsigned short*)(buf + 2))
			{
				if (write(fd, buf + 4, recv_len - 4) < 0)
				{
					error.err("write error");
					close(fd);
					break;
				}
			}

			buf[1] = 4;
			if (sendto(cfd, buf, 4, 0, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
			{
				error.err("sendto error");
				close(fd);
				return;
			}

			if (recv_len < BUFFER_SIZE)
			{
				std::cout << "-----�ļ��������" << std::endl;
				close(fd);
				break;
			}
			num++;
		}
		else if (buf[1] == 5)
		{
			error.err("_____error: " + std::string(buf + 4) + "_____");
			if (flag == 1)
			{
				close(fd);
			}
			break;
		}

	}
}

void TFTPClient::execUpload()
{
	std::string filename;
	std::cout << "������Ҫ�ϴ����ļ���: ";
	std::getline(std::cin, filename);

	int fd = open(filename.c_str(), O_RDONLY);
	if (fd < 0)
	{
		if (errno == ENOENT)
		{
			error.err("�ļ�������");
			return;
		}
		error.err("open error");
		return;
	}

	char buf[BUFFER_SIZE] = "";
	int size = sprintf(buf, "%c%c%s%c%s%c", 0, 2, filename.c_str(), 0, "octet", 0);

	if (sendto(cfd, buf, size, 0, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
	{
		error.err("sendto error");
		close(fd);
		return;
	}

	int recv_len;
	unsigned short num = 0;
	socklen_t addrlen = sizeof(server_addr);

	while (true)
	{
		memset(buf, 0, sizeof(buf));
		recv_len = recvfrom(cfd, buf, BUFFER_SIZE, 0, (sockaddr*)&server_addr, &addrlen);
		if (recv_len < 0)
		{
			error.err("recvfrom error");
			close(fd);
			return;
		}

		if (buf[1] == 4)
		{
			if (num == ntohs(*(unsigned short*)(buf + 2)))
			{
				buf[1] = 3;
				num++;
				*(unsigned short*)(buf + 2) = htons(num);
			}

			int res = read(fd, buf + 4, BUFFER_SIZE - 4);
			if (res < 0)
			{
				error.err("read error");
				close(fd);
				break;
			}
			else if (res == 0)
			{
				std::cout << "-----�ϴ����-----" << std::endl;
				break;
			}

			if (sendto(cfd, buf, BUFFER_SIZE, 0, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
			{
				error.err("sendto error");
				close(fd);
				return;
			}
		}
		else if (buf[1] == 5)
		{
			error.err("�ϴ�ʧ��");
			break;
		}
	}
}

void TFTPClient::clearScream()
{
	std::cout << "�����������";
	while (getchar() != '\n');
}

void TFTPClient::waitForInput()
{
	while (getchar() != '\n');
}

void TFTPClient::showMenu()
{
	system("cls");
	std::cout << "******************����UDP��TFTP�ļ�����********************" << std::endl;
	std::cout << "*********************1������************************" << std::endl;
	std::cout << "*********************2���ϴ�************************" << std::endl;
	std::cout << "*********************3���˳�************************" << std::endl;
	std::cout << "**********************************************************" << std::endl;
}

void TFTPClient::run()
{
	while (true)
	{
		showMenu();

		char choice;
		std::cin >> choice;
		waitForInput();

		switch (choice)
		{
		case '1':
			execDownload();
			break;
		case '2':
			execUpload();
			break;
		case '3':
			return;
		default:
			std::cout << "��������" << std::endl;
			break;
		}

		clearScream();
	}
}
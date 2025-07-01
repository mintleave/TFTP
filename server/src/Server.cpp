#include "Server.h"

TFTPServer::TFTPServer()
{
	INIT_SOCKET();

	// 填充地址信息结构体
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
#if defined(_WIN32) || defined(_WIN64)
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
#else 
	addr.sin_addr.s_addr = INADDR_ANY;
#endif 

	// 创建UDP套接字
	sfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sfd < 0)
	{
		log.error("socker creation failed");
		return;
	}
	log.info("socker creation success");

	// 设置端口重用
	int opt = 1;
	int u = setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR,
#if defined(_WIN32) || defined(_WIN64)
		(const char*)&opt,
#else
		& opt,
#endif
		sizeof(opt));
	if (u < 0)
	{
		log.error("setsockopt error");
		return;
	}
	log.error("setsockopt success");

	// 绑定
	if (bind(sfd, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) 
	{
		log.error("bind error");
		return;
	}
	log.info("bind success");
}

TFTPServer::~TFTPServer()
{
	CLOSE_SOCKET(sfd);
	CLEANUP_SOCKET();
	log.info("server end !!!");
}

void TFTPServer::run()
{
	log.info(std::string("server start on: ") + std::to_string(PORT));
	log.info(std::string("serving file from: ") + root_dir);

	char buf[BUFFER_SIZE] = "";
	sockaddr_in client_addr;
	socklen_t addr_len = sizeof(client_addr);

	while (true)
	{
		int n = recvfrom(sfd, buf, BUFFER_SIZE, 0, (sockaddr*)&client_addr, &addr_len);
		if (n < 0)
		{
			log.error("recvfrom error");
			return;
		}

		if (buf[0] != 0) continue;

		// 解析请求文件名
		char* filename = buf + 2;
		// 解析协议模式
		const char* mode = filename + strlen(filename) + 1;

		if (strcasecmp(mode, "octet") != 0)
		{
			sendError("Only binary mode supported", client_addr, addr_len);
			continue;
		}

		switch (buf[1])
		{
		case 1:	// 读请求
			log.info(std::string("read request for : ") + std::string(filename));
			handleReadRequest(filename, client_addr, addr_len);
			break;
		case 2: // 写请求
			log.info(std::string("write request for : ") + std::string(filename));
			handleWriteRequest(filename, client_addr, addr_len);
		default: // 异常请求
			log.info(std::string("Unknown request from : ") + std::to_string(client_addr.sin_addr.s_addr));
			sendError("Unknow request", client_addr, addr_len);
			break;
		}
	}
}

void TFTPServer::handleReadRequest(const char* filename, struct sockaddr_in& client_addr, socklen_t addr_len)
{
	// 要下载文件的全路径
	std::string full_path = root_dir + "/" + filename;
	// 以只读形式打开文件
	int fd = open(full_path.c_str(), O_RDONLY);
	if (fd < 0)
	{
		sendError("File not found", client_addr, addr_len);
		return;
	}

	char buf[BUFFER_SIZE] = "";
	unsigned short block_num = 1;

	while (true)
	{
		buf[0] = 0;
		buf[1] = 3;
		*(unsigned short*)(buf + 2) = htons(block_num);

		// 读文件
		int n = read(fd, buf + 4, BUFFER_SIZE - 4);
		if (n < 0)
		{
			sendError("Read error", client_addr, addr_len);
			close(fd);
			return;
		}

		// 发送数据包
		if (sendto(sfd, buf, n + 4, 0, (sockaddr*)&client_addr, addr_len) < 0)
		{
			log.error("sendto error");
			close(fd);
			return;
		}

		do
		{
			// 接收客户端发来的ACK数据包
			if (recvfrom(sfd, buf, BUFFER_SIZE, 0, (sockaddr*)&client_addr, &addr_len) < 0)
			{
				log.error("recvfrom error");
				close(fd);
				return;
			}
		} while (buf[1] != 4 || ntohs(*(unsigned short*)(buf + 2)) != block_num);

		if (n < BUFFER_SIZE - 4)
		{
			break;
		}

		block_num++;
	}

	close(fd);
}

void TFTPServer::handleWriteRequest(const char* filename, struct sockaddr_in& client_addr, socklen_t addr_len)
{
	// 获取文件全路径
	std::string full_path = root_dir + "/" + filename;
	// 以只写的形式打开文件
	int fd = open(full_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0664);
	if (fd < 0)
	{
		sendError("Cannot create file", client_addr, addr_len);
		return;
	}

	// ACK
	char buf[BUFFER_SIZE] = "";
	unsigned short block_num = 0;

	buf[0] = 0;
	buf[1] = 4;
	*(unsigned short*)(buf + 2) = htons(block_num);

	// 发送ACK
	if (sendto(sfd, buf, BUFFER_SIZE, 0, (sockaddr*)&client_addr, addr_len) < 0)
	{
		log.error("sendto error");
		return;
	}

	// 循环接收数据包
	while (true)
	{
		// 读取数据包
		int n = recvfrom(sfd, buf, BUFFER_SIZE, 0, (sockaddr*)&client_addr, &addr_len);
		if (n < 0)
		{
			log.error("recvfrom error");
			close(fd);
			return;
		}

		if (buf[1] == 3 && ntohs(*(unsigned short*)(buf + 2)) == block_num + 1)
		{
			// 写入文件
			if (write(fd, buf + 4, n - 4) < 0)
			{
				sendError("Write error", client_addr, addr_len);
				close(fd);
				return;
			}
		}

		block_num++;
		buf[0] = 0;
		buf[1] = 4;
		*(unsigned short*)(buf + 2) = htons(block_num);
		// 发送ACK
		if (sendto(sfd, buf, BUFFER_SIZE, 0, (sockaddr*)&client_addr, addr_len) < 0)
		{
			log.error("sendto error");
			return;
		}

		if (n < BUFFER_SIZE)
		{
			break;
		}
	}

	close(fd);
}

void TFTPServer::sendError(const char* msg, struct sockaddr_in& client_addr, socklen_t addr_len)
{
	char buf[BUFFER_SIZE] = "";

	buf[0] = 0;
	buf[1] = 5;
	buf[2] = 0;
	buf[3] = 1;
	strcpy(buf + 4, msg);

	if (sendto(sfd, buf, strlen(msg) + 5, 0, (sockaddr*)&client_addr, addr_len) < 0)
	{
		log.error("sendto error");
		return;
	}
}
#pragma once
#include "ThreadPool.h"
#include "log.h"
#include "Macros.h"

class TFTPServer
{
public:
	TFTPServer();
	~TFTPServer();

	// 启动服务器
	void run();
private:
	// 服务器地址信息结构体
	sockaddr_in addr;
	// 服务器套接字文件描述符
	SOCKET_TYPE sfd;
	// 文件服务的根目录
	std::string root_dir = ".";
	// 数据包的最大容量
	static constexpr int BUFFER_SIZE = 516;
	// 端口
	static constexpr int PORT = 7788;

	// 日志系统
	Logger log;

	/**
	* 功能: 处理客户端的请求函数
	* 参数:
	*	filename: 发送文件的文件名
	*	client:	客户端地址信息结构体
	*	addr_len: 地址信息结构体的大小
	*/
	void handleReadRequest(const char* filename, struct sockaddr_in& client, socklen_t addr_len);

	/**
	* 功能: 处理客户端的请求函数
	* 参数:
	*	filename: 存储接收数据的文件名
	*	client:	客户端地址信息结构体
	*	addr_len: 地址信息结构体的大小
	*/
	void handleWriteRequest(const char* filename, struct sockaddr_in& client, socklen_t addr_len);

	/**
	* 功能: 发送给客户端错误信息
	* 参数:
	*	msg: 要发送给客户端的错误信息
	*	client:	客户端地址信息结构体
	*/
	void sendError(const char* msg, struct sockaddr_in& client, socklen_t addr_len);
};
#pragma once

#if LINUX
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <strings.h>
#include <fcntl.h>
#include <unistd.h>

#define SOCKET_ERROR -1

#define CLOSE_SOCKET close
#define SOCKET_TYPE int
#define INIT_SOCKET()
#define CLEANUP_SOCKET()

#elif defined(_WIN32) || defined(_WIN64)

#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#define SOCKET_TYPE SOCKET
#define CLOSE_SOCKET closesocket
#define INIT_SOCKET() \
        WSADATA wsaData; \
        if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) { \
            log.error("WSAStartup failed"); \
        }
#define CLEANUP_SOCKET() WSACleanup()

#include <cstring>
#define strcasecmp _stricmp
#include <io.h>
#include <fcntl.h>

#endif
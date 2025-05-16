#ifndef WEBSERV_HPP
#define WEBSERV_HPP

#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#include <cstring>
#include <cerrno>
#include <ctime>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <list>
#include <map>
#include <vector>

#define WS_MAX_CLIENT_MAX_BODY_SIZE 104857600
#define WS_MAX_URI_SIZE 6144
#define WS_CLIENT_HEADER_BUFFER_SIZE 8192 // 4 max
#define WS_CLIENT_BODY_BUFFER_SIZE 16384
#define WS_EPOLL_NB_EVENTS 512
#define WS_BACKLOG 511
#define WS_SENDER_BUFFER_SIZE 32768
#define CRLF "\r\n"

enum Method
{
	GET,
	POST,
	DELETE
};

#endif

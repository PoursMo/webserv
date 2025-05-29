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
#include <dirent.h>
#include <sys/wait.h>

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
#include <csignal>

#define WS_MAX_CLIENT_MAX_BODY_SIZE 104857600 // Maximum allowed clientMaxBodySize
#define WS_MAX_URI_SIZE 6144				  // Maximum allowed URI size
#define WS_CLIENT_HEADER_BUFFER_SIZE 8192	  // Client header buffer size, max 4
#define WS_CLIENT_BODY_BUFFER_SIZE 16384	  // Client body buffer size
#define WS_EPOLL_NB_EVENTS 512				  // Epoll maximum number of events returned by epoll_wait
#define WS_BACKLOG 511						  // Listening socket backlog size
#define WS_SENDER_BUFFER_SIZE 32768			  // Sender buffer size
#define WS_CONNECTION_TIMEOUT_TIMER 60		  // Time for connection timeout
#define CRLF "\r\n"							  // CRLF

enum Method
{
	GET,
	POST,
	DELETE
};

#endif

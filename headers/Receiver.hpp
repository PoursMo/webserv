#ifndef RECEIVER_HPP
#define RECEIVER_HPP

#include "http_error.hpp"

#include <cstring>
#include <cerrno>
#include <algorithm>

#include <list>
#include <sys/socket.h>

#include <iostream>

#define WS_CLIENT_HEADER_BUFFER_SIZE 10 // 4 max
// #define WS_CLIENT_HEADER_BUFFER_SIZE 8192 // 4 max
#define WS_CLIENT_BODY_BUFFER_SIZE 16384

struct Buffer
{
	const char *first;
	const char *last;
	const char *pos;
	size_t capacity;
};

const char *ws_strchr(const char *first, const char *last, char c)
{
	while (1)
	{
		if (*first == c)
			return first;
		if (first == last)
			break;
		first++;
	}
	return 0;
}

void debug_print(const char *first, const char *last)
{
	while (1)
	{
		std::cout << *first;
		if (first == last)
			break;
		first++;
	}
	std::cout << std::endl;
}

class Receiver
{
private:
	int fd;
	// request object reference
	std::list<Buffer *> buffers;
	size_t headerBufferCount;
	// size_t bodyBytesRecvd = 0;

public:
	Receiver()
	{
	}

	Receiver(int fd) : fd(fd), headerBufferCount(0)
	{
	}

	void sendLines()
	{
		for (std::list<Buffer *>::iterator i = buffers.begin(); i != buffers.end(); i++)
		{
			const char *nl;
			while ((nl = ws_strchr((*i)->pos, (*i)->last, '\n')))
			{
				if (i != buffers.begin())
				{
					std::string line;
					for (std::list<Buffer *>::iterator j = buffers.begin(); j != i;)
					{
						line.append((*j)->pos, (*j)->last);
						std::list<Buffer *>::iterator tmp = j;
						++j;
						delete (*tmp)->first;
						delete (*tmp);
						buffers.erase(tmp);
					}
					line.append((*i)->pos, nl);
					debug_print(&line[0], &line[line.size() - 1]);
					// if (!std::strcmp(line.c_str(), "\r\n\r\n"))
					// is end of header
					// request.line(&line[0], &line[line.size()])
				}
				else
				{
					debug_print((*i)->pos, nl);
					// if (!std::strcmp((*i)->pos, "\r\n\r\n"))
					// is end of header
					// request.line(pos, nl)
				}
				if (nl != (*i)->last)
					(*i)->pos = nl + 1;
				else // edge case where \n is exactly at the end of buffer
				{
					std::cout << "blbl" << std::endl;
					std::list<Buffer *>::iterator tmp = i;
					++i;
					delete (*tmp)->first;
					delete (*tmp);
					buffers.erase(tmp);
					break;
				}
			}
		}
	}

	void recvHeader() // header
	{
		if (headerBufferCount >= 4)
			throw http_error(414);
		Buffer *buffer;
		buffer = buffers.back();
		char *tmp;
		if (!buffers.empty())
			std::cout << "buffer->last - buffer->first: " << buffer->last - buffer->first << std::endl;
		if (buffers.empty() || (size_t)(buffer->last - buffer->first) == buffer->capacity)
		{
			buffer = new Buffer;
			tmp = new char[WS_CLIENT_HEADER_BUFFER_SIZE];
			buffer->capacity = WS_CLIENT_HEADER_BUFFER_SIZE;
			buffer->first = tmp;
			buffer->pos = tmp;
			ssize_t bytes_received = recv(fd, tmp, WS_CLIENT_HEADER_BUFFER_SIZE, 0);
			if (bytes_received == -1)
				throw http_error(std::strerror(errno), 500); // free ?
			if (bytes_received == 0)
			{
				std::cout << "Client with fd: " << fd << " disconnected" << std::endl;
				// handle disconnection
			}
			buffer->last += bytes_received;
			buffers.push_back(buffer);
		}
		else
		{
			tmp = (char *)buffer->last + 1;
			size_t readSize = buffer->capacity - (buffer->last - buffer->first);
			ssize_t bytes_received = recv(fd, tmp, readSize, 0);
			if (bytes_received == -1)
				throw http_error(std::strerror(errno), 500);
			if (bytes_received == 0)
			{
				std::cout << "Client with fd: " << fd << " disconnected" << std::endl;
				// handle disconnection
			}
			buffer->last += bytes_received;
		}
		sendLines();
	}

	void receive()
	{
		recvHeader();
	}
};

#endif
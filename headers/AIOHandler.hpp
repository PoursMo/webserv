#ifndef AIOHANDLER_HPP
#define AIOHANDLER_HPP

#include "webserv.hpp"

class Poller;

class AIOHandler
{
    protected:
    	struct Buffer
		{
			char *first;
			char *last;
			char *pos;
			size_t capacity;
		};
		enum BufferType
		{
			HEADER,
			BODY
		};

        int inputFd;
        int outputFd;
        Poller &poller;
    	std::list<Buffer *> buffers;
        
        AIOHandler(Poller &poller);
        ~AIOHandler();
        static void debugPrint(const char *first, const char *const last);
    public:
        virtual bool handleInput() = 0;
        virtual bool handleOutput() = 0;
        void setInputFd(int fd);
        void setOutputFd(int fd);
};

#endif

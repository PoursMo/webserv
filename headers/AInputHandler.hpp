#ifndef AIPUTHANDLER_HPP
#define AIPUTHANDLER_HPP

#include "webserv.hpp"

class AInputHandler
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

		int inputFd; //formerly clientFd;
		void *parser; //formerly request;
		std::list<Buffer *> buffers;
		bool readingHeader;
		ssize_t bytesInput;

		virtual void sendHeaderLineToParsing(const Buffer *lbuffer, char *lf) = 0;
		virtual void flushHeaderBuffers() = 0;
		virtual void getInput(void *buf, size_t len) = 0;
		virtual Buffer *createBuffer(BufferType type) = 0;
		virtual bool fillBuffer(BufferType type) = 0;
		AInputHandler(int inputFd, void *parser);
		~AInputHandler();
	public:
		virtual bool handleInput() = 0;
		ssize_t getBytesInput() const;
		//Utils
		static char *ws_strchr(char *first, const char *const last, char c);
};

#endif

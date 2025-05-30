#ifndef AINPUTHANDLER_HPP
#define AINPUTHANDLER_HPP

#include "AIOHandler.hpp"
#include "webserv.hpp"

class AInputHandler: virtual public AIOHandler
{
	private:
		static char *ws_strchr(char *first, const char *const last, char c);

		void sendHeaderLineToParsing(const Buffer *lbuffer, char *lf);
		void addBodyBytes(size_t bytes);
		void flushHeaderBuffers();
		Buffer *createBuffer(BufferType type);
		bool fillBuffer(BufferType type);
		
	protected:
	    int inputFd;
		bool isReadingHeader;
		ssize_t bytesInput;
		ssize_t bodyBytesCount;
		size_t headerBufferCount;

		virtual bool isInputEnd() = 0;
		virtual void onInputEnd() = 0;
		virtual bool parseLine(char *lstart, char *lend) = 0;
		virtual void onUpdateBodyBytes() = 0;
		virtual ssize_t handleInputSysCall(void *buf, size_t len) = 0;
		virtual void onHeaderBufferCreation() = 0;

		AInputHandler();
		~AInputHandler();
	public:
		void handleInput();
		void setInputFd(int fd);
		void delInputFd();
		//Utils

		bool isInputRegularFile;
	};

#endif

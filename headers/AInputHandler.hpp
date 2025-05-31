#ifndef AINPUTHANDLER_HPP
#define AINPUTHANDLER_HPP

#include "AIOHandler.hpp"
#include "webserv.hpp"

class AInputHandler : virtual public AIOHandler
{
private:
	static char* ws_strchr(char* first, const char* const last, char c);

	void sendHeaderLineToParsing(const Buffer* lbuffer, char* lf);
	void addBodyBytes(size_t bytes);
	void flushHeaderBuffers();
	Buffer* createBuffer(BufferType type);
	bool fillBuffer(BufferType type);

protected:
	bool isReadingHeader;
	ssize_t bytesInput;
	ssize_t bodyBytesCount;
	size_t headerBufferCount;

	virtual bool parseLine(char* lstart, char* lend) = 0;
	virtual void onUpdateBodyBytes() = 0;
	virtual void onHeaderBufferCreation() = 0;

	AInputHandler();
	virtual ~AInputHandler();
public:
	void handleInput();
	void subscribeInputFd(int fd);
	void unsubscribeInputFd();
	int getInputFd();
};

#endif

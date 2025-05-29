#ifndef AOUTPUTHANDLER_HPP
#define AOUTPUTHANDLER_HPP

#include "AIOHandler.hpp"
#include "webserv.hpp"

class AOutputHandler : virtual public AIOHandler
{
	private:
		std::string stringContent;

	protected:
		ssize_t bytesOutputCount;

		virtual ssize_t handleOutputSysCall(const void *buf, size_t len) = 0;
		virtual bool isOutputEnd() = 0;
		virtual void onOutputEnd() = 0;
		
	public:
		AOutputHandler();
		void handleOutput();
};

#endif

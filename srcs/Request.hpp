#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <string>
#include "LocationData.hpp"
#include "http_error.hpp"

class Request
{
	private:
		Method method;
		std::string resource;
		std::map<std::string, std::string> headers;
		int bodyFd = -1;
		bool firstLineParsed = false;
		void parseFirstLine(char *lstart, char *lend);
		Method setMethod(char *lstart, char *lend);
		std::string setResource(char **lstart, char *lend);
		void parseHeaderLine(char *lstart, char *lend);
		void addHeader();

	public:
		Request();
		~Request();
		void parseRequestLine(char *lstart, char *lend);
		void setBodyFd();
		void execute();

	class UnrecognizedMethod: public std::exception
	{
		const char *what() const throw();
	};

	class UnsupportedMethod: public std::exception
	{
		const char *what() const throw();
	};
};

#endif

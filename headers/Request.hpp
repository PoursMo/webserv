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
		int bodyFd;
		int socketFd;
		bool error;
		unsigned long contentLength;
		bool firstLineParsed;
		bool headerParsed;
		void RequestError(int code);
		void parseFirstLine(char *lstart, char *lend);
		bool checkEmptyline(char *lstart, char *lend);
		Method setMethod(char *lstart, char *lend);
		std::string setResource(char **lstart, char *lend);
		void parseHeaderLine(char *lstart, char *lend);
		void addHeader(std::string key, std::string value);

	public:
		Request();
		~Request();
		void parseRequestLine(char *lstart, char *lend);
		std::string getResource() const;
		enum Method getMethod() const;
		std::string getHeaderValue(const std::string key) const;

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

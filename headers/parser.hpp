#ifndef PARSER_HPP
#define PARSER_HPP

#include "webserv.hpp"
#include "http_error.hpp"
#include "Request.hpp"

namespace parser {
	void trimWhitespaceAfter(std::string &value);
	bool isEmptyline(char *lstart, char *lend);
	void parseHeaderLine(void (AIOHandler::*addHeader)(std::string, std::string), char *lstart, char *lend);
}

#endif

#include "parser.hpp"

void parser::trimWhitespaceAfter(std::string &value)
{
	value.erase(value.find_last_not_of(" \t") + 1);
}

bool parser::isEmptyline(char *lstart, char *lend)
{
	if (*lstart != '\r' && *lstart != '\n')
		return false;
	if (lstart == lend && *lstart == '\n')
		return true;
	lstart++;
	if (lstart == lend && *lstart == '\n')
		return true;
	return false;
}

void parser::parseHeaderLine(void (*addHeader)(std::string, std::string), char *lstart, char *lend)
{
	std::string key;
	std::string value = "";

	while (lstart != lend && *lstart != '\r' && *lstart != ':' && *lstart != ' ' && *lstart != '\t')
	{
		key = key + *lstart;
		lstart++;
	}
	if (*lstart != ':')
		throw http_error("No ':' in header line", 400);
	lstart++;
	while (*lstart == ' ' || *lstart == '\t')
		lstart++;
	while (lstart != lend && *lstart != '\r')
	{
		if (!std::isprint(*lstart) && *lstart != '\t')
			throw http_error("Invalid character in header value", 400);
		value = value + *lstart;
		lstart++;
	}
	trimWhitespaceAfter(value);
	if (*lstart != '\r' && lstart != lend)
		throw http_error("Header line not correctly ended", 400);
	if (*lstart == '\r' && *(lstart + 1) != '\n')
		throw http_error("Header line not correctly ended", 400);
	addHeader(key, value);
}

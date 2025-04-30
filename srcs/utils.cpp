#include "utils.hpp"

#include <algorithm>
#include <stdexcept>

std::string int_to_str(int num)
{
	bool isNegative = (num < 0);
	if (isNegative)
		num = -num;
	std::string result;
	do
	{
		result += '0' + (num % 10);
		num /= 10;
	} while (num > 0);
	if (isNegative)
		result += '-';
	std::reverse(result.begin(), result.end());
	return result;
}

int extract_status_code(const std::string &s)
{
	for (std::string::const_iterator i = s.begin(); i != s.end(); i++)
	{
		if (!std::isdigit(*i))
			throw std::runtime_error("Invalid status code.");
	}
	char *end;
	unsigned long code = std::strtoul(s.c_str(), &end, 10);
	if (*end != '\0' || code < 300 || code > 599)
		throw std::runtime_error("Invalid status code.");
	return code;
}
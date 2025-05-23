#include "utils.hpp"

std::string int_to_str(int num, const std::string& base)
{
	bool isNegative = (num < 0);
	if (isNegative)
		num = -num;
	std::string result;
	do
	{
		int digit = num % base.length();
		result += base[digit];
		num /= base.length();
	} while (num > 0);
	if (isNegative)
		result += '-';
	std::reverse(result.begin(), result.end());
	return result;
}

std::string int_to_str(int num)
{
	std::string base = "0123456789";
	bool isNegative = (num < 0);
	if (isNegative)
		num = -num;
	std::string result;
	do
	{
		int digit = num % base.length();
		result += base[digit];
		num /= base.length();
	} while (num > 0);
	if (isNegative)
		result += '-';
	std::reverse(result.begin(), result.end());
	return result;
}

std::string long_to_str(long num)
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

std::string ulong_to_str(unsigned long num)
{
	std::string result;
	do
	{
		result += '0' + (num % 10);
		num /= 10;
	} while (num > 0);
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

std::string &str_to_lower(std::string &str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		str[i] = std::tolower(str[i]);
	}
	return str;
}

std::string getDateString()
{
	std::time_t rawtime;
	std::tm *timeinfo;
	char buffer[80];

	std::time(&rawtime);
	timeinfo = std::gmtime(&rawtime);
	std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);

	std::string formatted_time(buffer);
	return (formatted_time);
}

std::string getDateString(time_t time)
{
	char buffer[80];

	tm *timeinfo = std::gmtime(&time);
	std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);

	std::string formatted_time(buffer);
	return (formatted_time);
}

int str_to_int(const std::string str, const std::string& base, int value)
{
	std::size_t digit = base.find(str[0]);
	if (digit == std::string::npos)
		return value;
	return (str_to_int(str.data() + 1, base, (value * base.length()) + digit));
}

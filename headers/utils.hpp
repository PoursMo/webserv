#ifndef UTILS_HPP
#define UTILS_HPP

#include "webserv.hpp"

std::string int_to_str(int num);
std::string long_to_str(long num);
std::string ulong_to_str(unsigned long num);

int extract_status_code(const std::string &s);

std::string &str_to_lower(std::string &str);

std::string getDateString();

std::string getDateString(time_t time);

template <typename T>
bool isInVector(const std::vector<T> &vector, const T &value)
{
	for (typename std::vector<T>::const_iterator i = vector.begin(); i != vector.end(); i++)
	{
		if (*i == value)
			return true;
	}
	return false;
}

#endif

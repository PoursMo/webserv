#ifndef URI_HPP
#define URI_HPP

#include "webserv.hpp"
#include "utils.hpp"

std::string decodeUri(const std::string &src);
std::string encodeUri(const std::string &src);

#endif

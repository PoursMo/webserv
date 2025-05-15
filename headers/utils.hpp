#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>

std::string int_to_str(int num);

int extract_status_code(const std::string &s);

std::string &str_to_lower(std::string &str);

#endif
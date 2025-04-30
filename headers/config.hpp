#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "ft_json.hpp"

class Server;

int extract_status_code(const std::string &s);
std::vector<Server *> create_servers(const ft_json::JsonValue &json);

#endif
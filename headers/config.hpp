#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "ft_json.hpp"

class Server;

std::vector<Server *> create_servers(const ft_json::JsonValue &json);

#endif
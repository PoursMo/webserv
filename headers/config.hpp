#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "ft_json.hpp"

class VirtualServer;

std::map<int, std::vector<VirtualServer *> > create_servers(const ft_json::JsonValue &json);

#endif
#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "ft_json.hpp"

class VirtualServer;

void create_servers(const ft_json::JsonValue &json, std::map<int, std::vector<VirtualServer *> > &servers);

#endif
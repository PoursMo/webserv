#include "webserv.hpp"
#include "config.hpp"
#include "VirtualServer.hpp"
#include "Poller.hpp"
#include "http_status.hpp"
#include "Logger.hpp"

Logger logger;

bool init(const char *config_path, std::map<int, std::vector<VirtualServer *> > &servers)
{
	std::ifstream file(config_path);
	if (!file)
	{
		std::cerr << config_path << ": Could not open file " << std::endl;
		return false;
	}
	try
	{
		ft_json::JsonValue json = ft_json::parse_json(file);
		if (json.asObject().count("debug"))
			logger.setEnabled(json.asObject().at("debug").asBoolean());
		logger.log() << "\033[1m\033[31m" << "servers:" << "\033[0m" << std::endl;
		create_servers(json, servers);
		for (std::map<int, std::vector<VirtualServer *> >::iterator i = servers.begin(); i != servers.end(); i++)
		{
			logger.log() << "Socket: " << i->first << std::endl;
			for (std::vector<VirtualServer *>::const_iterator j = i->second.begin(); j != i->second.end(); j++)
			{
				logger.log() << *(*j);
			}
		}
		logger.log() << std::endl;
		http_status::init();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << '\n';
		return false;
	}
	return true;
}

int main(int argc, char **argv)
{
	const char *config_path = "conf/default.json";
	if (argc > 1)
		config_path = argv[1];
	std::map<int, std::vector<VirtualServer *> > servers;
	if (!init(config_path, servers))
		return 1;

	logger.log() << "\033[1m\033[31m" << "poll loop:" << "\033[0m" << std::endl;
	Poller poller(servers);
	poller.loop();

	return 0;
}

#include "VirtualServer.hpp"
#include "config.hpp"
#include "utils.hpp"

static int initializeSocket(in_port_t port, in_addr_t vserver_addr)
{
	int socket_fd = socket(PF_INET, SOCK_STREAM, 0); // SOCK_NONBLOCK ? it has no effect on epoll
	if (socket_fd == -1)
		throw std::runtime_error("socket: " + std::string(strerror(errno)));

	// Reuse of local addresses is supported
	int opt = 1;
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(socket_fd);
		throw std::runtime_error("setsockopt: " + std::string(strerror(errno)));
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = htonl(vserver_addr);
	memset(server_addr.sin_zero, 0, sizeof(server_addr.sin_zero));
	if (bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1)
	{
		close(socket_fd);
		throw std::runtime_error("bind: " + std::string(strerror(errno)));
	}

	if (listen(socket_fd, WS_BACKLOG) == -1)
	{
		close(socket_fd);
		throw std::runtime_error("listen: " + std::string(strerror(errno)));
	}
	return socket_fd;
}

static int check_duplicate_fd(const std::map<int, std::vector<VirtualServer*> >& m, const VirtualServer* const server)
{
	for (std::map<int, std::vector<VirtualServer*> >::const_iterator i = m.begin(); i != m.end(); i++)
	{
		for (std::vector<VirtualServer*>::const_iterator j = i->second.begin(); j != i->second.end(); j++)
		{
			if ((*j)->getAddress() == server->getAddress() && (*j)->getPort() == server->getPort())
				return i->first;
		}
	}
	return -1;
}

void create_servers(const ft_json::JsonValue& json, std::map<int, std::vector<VirtualServer*> >& servers)
{
	const ft_json::JsonObject& root = json.asObject();
	if (root.count("servers") != 1 || root.at("servers").getType() != ft_json::ARRAY)
		throw std::runtime_error("Root level must have a \"servers\" array of objects as it's value.");
	int count = 1;
	const ft_json::JsonArray& json_servers = root.at("servers").asArray();
	for (ft_json::JsonArray::const_iterator json_server = json_servers.begin(); json_server != json_servers.end(); json_server++)
	{
		try
		{
			VirtualServer* server = new VirtualServer((*json_server).asObject());
			int fd = check_duplicate_fd(servers, server);
			if (fd == -1)
				servers[initializeSocket(server->getPort(), server->getAddressAsNum())].push_back(server);
			else
				servers[fd].push_back(server);
		}
		catch (const std::exception& e)
		{
			for (std::map<int, std::vector<VirtualServer*> >::iterator i = servers.begin(); i != servers.end(); i++)
			{
				for (std::vector<VirtualServer*>::const_iterator j = i->second.begin(); j != i->second.end(); j++)
				{
					delete (*j);
				}
			}
			throw std::runtime_error("server " + int_to_str(count) + ": " + e.what());
		}
		count++;
	}
	if (count == 1)
		throw std::runtime_error("Atleast 1 server is required.");
}
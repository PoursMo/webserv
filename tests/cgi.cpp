#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <cstdio>
#include "config.hpp"
#include "LocationData.hpp"
#include "VirtualServer.hpp"

class Request
{

	private:
		LocationData location;
		Method method;
		std::string resource;
		std::map<std::string, std::string> headers;
		int bodyFd;

		// ADDED

	public:
		typedef struct {
			LocationData location;
			Method method;
			std::string resource;
			std::map<std::string, std::string> headers;
			int bodyFd;
		}	t_req_init;
		Request(t_req_init config);
		~Request();
		

};

Request::Request(t_req_init init): location(init.location), method(init.method), resource(init.resource), headers(init.headers), bodyFd(init.bodyFd)
{
}
Request::~Request()
{

}

void cgiExecution(const Request& req, int fd_out)
{
	char** env;
	(void)req;
	(void)env;

	//Build env


	return (0); // fd out
}

int main(int argc, char **argv)
{
	std::string config_path = "conf/cgi.json";
	std::map<int, std::vector<VirtualServer *> > servers;
	if (argc > 1)
		config_path = argv[1];
	std::ifstream file(config_path.c_str());
	if (!file)
	{
		std::cerr << config_path << ": Could not open file " << std::endl;
		return 1;
	}
	try
	{
		ft_json::JsonObject json = ft_json::parse_json(file).asObject();
		ft_json::JsonObject json_location = json
			.at("servers").asArray().at(0).asObject()
			.at("locations").asArray().at(0).asObject();
		LocationData location(json_location);
		std::cout << "location:" << location << std::endl;

		std::map<std::string, std::string> headers;
		headers["Host"] = "localhost";
		headers["Content-lenght"] = "42";

		Request req((Request::t_req_init) {
			.location = location,
			.method = GET,
			.resource = "/index.html",
			.headers = headers,
			.bodyFd = -1,
		});

		std::FILE* outFile = fopen("./logs/cgi-out.log", "w"); 
		outFile.
		cgiExecution(req);



	}
	catch (const std::exception &e)
	{
		std::cerr << "ERROR:" << e.what() << '\n';
		return 1;
	}
}
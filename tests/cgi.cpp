#include <map>
#include <string>
#include <string.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#include <fcntl.h>
#include <unistd.h>
#include "config.hpp"
#include "LocationData.hpp"
#include "VirtualServer.hpp"

class CgiHandler
{
	private:
		LocationData location;
		VirtualServer server;
		Method method;
		std::string resource;
		std::string query;
		std::map<std::string, std::string> headers;
		int bodyFd;
		std::string cgiExtension;
		std::string cgiPath;
		std::string cgiPathInfo;
		char** getCgiEnv();
		char** getCgiArgv();
		void initQuery();
		void initCgi();
	public:
		typedef struct {
			LocationData location;
			VirtualServer server;
			Method method;
			std::string resource;
			std::map<std::string, std::string> headers;
			int bodyFd;
		}	t_req_init;
		CgiHandler(t_req_init init);
		~CgiHandler();
		std::string getMethodString() const;
		const std::string getHeader(const std::string&) const;
		bool isCgiResource();
		pid_t cgiExecution(int fd_out);
};

CgiHandler::CgiHandler(t_req_init init):
	location(init.location),
	server(init.server),
	method(init.method),
	resource(init.resource),
	query(""),
	headers(init.headers),
	bodyFd(init.bodyFd),
	cgiExtension(""),
	cgiPath(""),
	cgiPathInfo("")
{
	this->initQuery();
	this->initCgi();
}

void CgiHandler::initQuery()
{
	std::size_t query_index = this->resource.find("?");
	if (query_index != std::string::npos)
	{
		this->query = this->resource.substr(query_index + 1);
		this->resource = this->resource.substr(0, query_index);
	}
}

void CgiHandler::initCgi()
{
	std::map<std::string, std::string> cgiConfig = this->location.getCgiConfig();
	for (std::map<std::string, std::string>::const_iterator it = cgiConfig.begin(); it != cgiConfig.end(); it++)
	{
		std::string ext = it->first;
		std::size_t index = this->resource.find(ext); // TODO: findlast() ?
		if (index != std::string::npos)
		{
			std::size_t end = index + ext.length();
			if (!this->resource[end] || this->resource[end] == '/')
			{
				this->cgiPathInfo = this->resource.substr(end);
				this->resource = this->resource.substr(0, end);
				this->cgiExtension = it->first;
				this->cgiPath = it->second;
				return ;
			}
		}
	}
}

CgiHandler::~CgiHandler()
{
}

std::string CgiHandler::getMethodString() const
{
	if (this->method == GET)
		return "GET";
	if (this->method == POST)
		return "POST";
	return "DELETE";
}

const std::string CgiHandler::getHeader(const std::string& key) const
{
	if (this->headers.count(key))
		return this->headers.at(key);
	return "";
}

bool CgiHandler::isCgiResource()
{
	return (this->cgiPath != "");
}

static void connect_fd(int a, int b)
{
	if (dup2(a, b) == -1)
	{
		close(a);
		throw std::runtime_error("dup2 failed");
	}
	close(a);
}

static char *ft_strdup(const char* src)
{
	char *dup = new char[strlen(src)];
	strcpy(dup, src);
	return dup;
}

static char **mapToStringArray(std::map<std::string, std::string> map)
{
	char **arr = new char*[map.size() + 1]();
	size_t index = 0;

	for (std::map<std::string, std::string>::const_iterator it = map.begin(); it != map.end(); it++)
	{
		std::string value =  it->first + "=" + it->second;
		arr[index++] = ft_strdup(value.c_str());
	}
	return (arr);
}

char **CgiHandler::getCgiEnv()
{
	std::map<std::string, std::string> env;

	// https://fr.wikipedia.org/wiki/Variables_d%27environnement_CGI
	env["SERVER_SOFTWARE"] = "webserv_42";
	env["SERVER_NAME"] = this->getHeader("Host");
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["REDIRECT_STATUS"] = "200";

	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["SERVER_PORT"] = this->server.getPort();
	env["REQUEST_METHOD"] = this->getMethodString();
	env["PATH_INFO"] = this->cgiPathInfo;
	env["PATH_TRANSLATED"] = this->location.getRoot() + this->resource;
	env["SCRIPT_NAME"] = this->resource;
	env["QUERY_STRING"] = this->query;
	// env["REMOTE_HOST"]
	// env["REMOTE_ADDR"]
	// env["AUTH_TYPE"]
	// env["REMOTE_USER"]
	// env["REMOTE_IDENT"]
	env["CONTENT_TYPE"] = this->getHeader("Content-type");
	env["CONTENT_LENGTH"] = this->getHeader("Content-Length");

	env["HTTP_ACCEPT"] = this->getHeader("Accept");
	env["HTTP_ACCEPT_LANGUAGE"] = this->getHeader("Accept-Language");
	env["HTTP_USER_AGENT"] = this->getHeader("User-Agent");
	env["HTTP_COOKIE"] = this->getHeader("Cookie");
	env["HTTP_REFERER"] = this->getHeader("Referer");

	return mapToStringArray(env);
}

char **CgiHandler::getCgiArgv()
{
	char **argv = new char*[3];

	argv[0] = ft_strdup(this->cgiPath.c_str());
	argv[1] = ft_strdup(std::string(this->location.getRoot() + this->resource).c_str());
	argv[2] = NULL;
	return argv;
}

pid_t CgiHandler::cgiExecution(int fd_out)
{

	int fd_in = this->bodyFd;

	if (this->cgiPath == "")
		throw std::runtime_error("cgi path is not defined");

	std::cout << "resource:\t" << this->resource << std::endl;
	std::cout << "query:\t\t" << this->query << std::endl;
	std::cout << "content-type:\t" << this->getHeader("Content-Type") << std::endl;
	std::cout << "content-length:\t" << this->getHeader("Content-Length") << std::endl;
	std::cout << "cgi extension:\t" << this->cgiExtension << std::endl;
	std::cout << "cgi path:\t" << this->cgiPath << std::endl;
	std::cout << "cgi pathinfo:\t" << this->cgiPathInfo << std::endl;

	pid_t pid = fork();
	if (pid)
	{
		close(fd_in);
		close(fd_out);
		return pid;
	}
	connect_fd(fd_in, STDIN_FILENO);
	close(fd_out);
	// connect_fd(fd_out, STDOUT_FILENO);
	char **envp = this->getCgiEnv();
	char **argv = this->getCgiArgv();
	if (execve(this->cgiPath.c_str(), argv, envp) == -1)
	{
		delete[] envp;
		delete[] argv;
		throw std::runtime_error("execv failed");
	}
	return 0;
}

int main(int argc, char **argv)
{
	std::string request_path = "./tests/http/data.http";
	std::map<int, std::vector<VirtualServer *> > servers;
	if (argc > 1)
		request_path = argv[1];
	try
	{
		std::ifstream file("conf/cgi.json");
		ft_json::JsonObject json = ft_json::parse_json(file).asObject();
		ft_json::JsonObject json_server = json.at("servers").asArray().at(0).asObject();
		ft_json::JsonObject json_location = json_server.at("locations").asArray().at(0).asObject();
		VirtualServer server(json_server);
		LocationData location(json_location);
		std::cout << "location:" << location << std::endl;

		std::map<std::string, std::string> headers;
		headers["Host"] = "localhost";
		headers["Content-Type"] = "application/json";
		headers["Content-Length"] = "20";

		int fd_in = open(request_path.c_str(), O_RDONLY);
		int fd_out = open("./logs/cgi-out.log", O_CREAT | O_TRUNC | O_WRONLY); 

		CgiHandler req((CgiHandler::t_req_init) {
			.location = location,
			.server = server,
			.method = POST,
			.resource = "/test.php/hahaha?name=toto",
			.headers = headers,
			.bodyFd = fd_in,
		});

		if (!req.isCgiResource())
		{
			std::cout << "CgiHandler don't match with CGI extension" << std::endl;
			return 1;
		}

		pid_t pid = req.cgiExecution(fd_out);
		std::cout << "TODO: wait timeout on :" << pid << " ?" << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cerr << "ERROR:" << e.what() << '\n';
		return 1;
	}
}
#include "CgiHandler.hpp"
#include "Request.hpp"
#include "VirtualServer.hpp"
#include "LocationData.hpp"
#include "http_error.hpp"
#include "Poller.hpp"
#include "Logger.hpp"

CgiHandler::CgiHandler(const Request &request) : request(request),
												 resource(request.getResource()),
												 query(""),
												 cgiExtension(""),
												 cgiPath(""),
												 cgiPathInfo(""),
												 fdIn(-1),
												 fdOut(-1)
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
	logger.log() << "Init CGI HANDLER" << std::endl;

	std::map<std::string, std::string> cgiConfig = this->request.getLocation()->getCgiConfig();
	for (std::map<std::string, std::string>::const_iterator it = cgiConfig.begin(); it != cgiConfig.end(); it++)
	{
		std::string ext = it->first;
		std::size_t index = this->resource.find(ext); // TODO: findlast() ?
		if (index != std::string::npos)
		{
			std::size_t end = index + ext.length();
			if (!this->resource[end] || this->resource[end] == '/')
			{
				// TODO: pathinfo = root + upload_store + path_info
				this->cgiPathInfo = this->resource.substr(end);
				this->resource = this->resource.substr(0, end);
				this->cgiExtension = it->first;
				this->cgiPath = it->second;
				return;
			}
		}
	}

	logger.log() << "RESOURCE IS NOT CGI:" << this->resource << std::endl;
}

std::string CgiHandler::getMethodString() const
{
	switch (this->request.getMethod())
	{
	case GET:
		return "GET";
		break;
	case POST:
		return "POST";
		break;
	default:
		return "DELETE";
		break;
	}
}

static void connect_fd(int fd, int stdFd)
{
	if (dup2(fd, stdFd) == -1)
	{
		close(fd);
		throw child_accident();
	}
	close(fd);
}

static char *ft_strdup(const char *src)
{
	char *dup = new char[strlen(src) + 1];
	strcpy(dup, src);
	return dup;
}

static char **mapToStringArray(std::map<std::string, std::string> map)
{
	char **arr = new char *[map.size() + 1];
	size_t index = 0;

	for (std::map<std::string, std::string>::const_iterator it = map.begin(); it != map.end(); it++)
	{
		std::string value = it->first + "=" + it->second;
		arr[index++] = ft_strdup(value.c_str());
	}
	arr[index] = NULL;
	return (arr);
}

static void deleteArray(char **arr)
{
	char **a = arr;
	while (*arr)
		delete[] *(arr++);
	delete[] a;
}

char **CgiHandler::getCgiEnv()
{
	std::map<std::string, std::string> env;

	// https://fr.wikipedia.org/wiki/Variables_d%27environnement_CGI
	env["SERVER_SOFTWARE"] = "webserv_42";
	env["SERVER_NAME"] = this->request.getHeaderValue("host");
	env["GATEWAY_INTERFACE"] = "CGI/1.1";
	env["REDIRECT_STATUS"] = "200";

	env["SERVER_PROTOCOL"] = "HTTP/1.1";
	env["SERVER_PORT"] = this->request.getVServer()->getPort();
	env["REQUEST_METHOD"] = this->getMethodString();
	env["PATH_INFO"] = this->cgiPathInfo;
	env["PATH_TRANSLATED"] = this->request.getLocation()->getRoot() + this->resource;
	env["SCRIPT_NAME"] = this->resource;
	env["QUERY_STRING"] = this->query;
	// env["REMOTE_HOST"]
	// env["REMOTE_ADDR"]
	// env["AUTH_TYPE"]
	// env["REMOTE_USER"]
	// env["REMOTE_IDENT"]
	env["CONTENT_TYPE"] = this->request.getHeaderValue("content-type");
	env["CONTENT_LENGTH"] = this->request.getHeaderValue("content-length");

	env["HTTP_ACCEPT"] = this->request.getHeaderValue("accept");
	env["HTTP_ACCEPT_LANGUAGE"] = this->request.getHeaderValue("accept-language");
	env["HTTP_USER_AGENT"] = this->request.getHeaderValue("user-agent");
	env["HTTP_COOKIE"] = this->request.getHeaderValue("cookie");
	env["HTTP_REFERER"] = this->request.getHeaderValue("referer");

	return mapToStringArray(env);
}

char **CgiHandler::getCgiArgv()
{
	char **argv = new char *[3];

	argv[0] = ft_strdup(this->cgiPath.c_str());
	argv[1] = ft_strdup(std::string(this->request.getLocation()->getRoot() + this->resource).c_str());
	argv[2] = NULL;
	return argv;
}

union t_pipe
{
	int fds[2];
	struct
	{
		int out;
		int in;
	};
};

int CgiHandler::cgiExecution()
{
	t_pipe pipefd_in;
	t_pipe pipefd_out;

	if (this->cgiPath == "")
		throw http_error("cgi path is not defined", 500);
	if (access(this->cgiPath.c_str(), X_OK) == -1)
		throw http_error("non existant cgi binary", 500);
	logger.log() << "resource:\t" << this->resource << std::endl;
	logger.log() << "query:\t\t" << this->query << std::endl;
	logger.log() << "content-type:\t" << this->request.getHeaderValue("content-type") << std::endl;
	logger.log() << "content-length:\t" << this->request.getHeaderValue("content-length") << std::endl;
	logger.log() << "cgi extension:\t" << this->cgiExtension << std::endl;
	logger.log() << "cgi path:\t" << this->cgiPath << std::endl;
	logger.log() << "cgi pathinfo:\t" << this->cgiPathInfo << std::endl;
	logger.log() << "method:\t" << this->getMethodString() << std::endl;

	if (pipe(pipefd_in.fds) == -1)
		throw http_error("pipe: " + std::string(strerror(errno)), 500);
	if (pipe(pipefd_out.fds) == -1)
		throw http_error("pipe: " + std::string(strerror(errno)), 500);

	int pid = fork();
	if (pid == -1)
		throw http_error("fork: " + std::string(strerror(errno)), 500);
	if (pid)
	{
		close(pipefd_in.out);
		close(pipefd_out.in);
		this->fdIn = pipefd_in.in;
		this->fdOut = pipefd_out.out;
		return pid;
	}
	this->request.getPoller().closeAll();
	close(pipefd_in.in);
	close(pipefd_out.out);
	connect_fd(pipefd_in.out, STDIN_FILENO);
	connect_fd(pipefd_out.in, STDOUT_FILENO);
	char **envp = this->getCgiEnv();
	char **argv = this->getCgiArgv();

	if (execve(this->cgiPath.c_str(), argv, envp) == -1)
	{
		deleteArray(envp);
		deleteArray(argv);
		throw child_accident();
	}
	return 0;
}

int CgiHandler::getFdIn() const
{
	return this->fdIn;
}

int CgiHandler::getFdOut() const
{
	return this->fdOut;
}

bool CgiHandler::isCgiResource() const
{
	return (this->cgiPath != "");
}

const char *child_accident::what() const throw()
{
	return "child error";
}

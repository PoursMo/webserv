#include "CgiHandler.hpp"
#include "Request.hpp"
#include "VirtualServer.hpp"
#include "LocationData.hpp"

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
	std::cout << "Init CGI HANDLER" << std::endl;

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

	std::cout << "RESOURCE IS NOT CGI:" << this->resource << std::endl;
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
		throw std::runtime_error("dup2:" + std::string(strerror(errno)));
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
		delete[] arr++;
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
		throw std::runtime_error("cgi path is not defined");

	std::cout << "resource:\t" << this->resource << std::endl;
	std::cout << "query:\t\t" << this->query << std::endl;
	std::cout << "content-type:\t" << this->request.getHeaderValue("content-type") << std::endl;
	std::cout << "content-length:\t" << this->request.getHeaderValue("content-length") << std::endl;
	std::cout << "cgi extension:\t" << this->cgiExtension << std::endl;
	std::cout << "cgi path:\t" << this->cgiPath << std::endl;
	std::cout << "cgi pathinfo:\t" << this->cgiPathInfo << std::endl;
	std::cout << "method:\t" << this->getMethodString() << std::endl;

	if (pipe(pipefd_in.fds) == -1)
		throw std::runtime_error("pipe: " + std::string(strerror(errno)));
	if (pipe(pipefd_out.fds) == -1)
		throw std::runtime_error("pipe: " + std::string(strerror(errno)));

	std::cout << "pipefd_in.in: " << pipefd_in.in << std::endl;
	std::cout << "pipefd_in.out: " << pipefd_in.out << std::endl;
	std::cout << "pipefd_out.in: " << pipefd_out.in << std::endl;
	std::cout << "pipefd_out.out: " << pipefd_out.out << std::endl;

	int pid = fork();
	if (pid == -1)
		throw std::runtime_error("fork: " + std::string(strerror(errno)));
	if (pid)
	{
		close(pipefd_in.out);
		close(pipefd_out.in);
		this->fdIn = pipefd_in.in;
		this->fdOut = pipefd_out.out;
		return pid;
	}
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
		throw execve_error();
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

const char *execve_error::what() const throw()
{
	return "execve failed";
}

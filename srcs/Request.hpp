#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <string>
#include "LocationData.hpp"

class Request
{
private:
    Method method;
    std::string resource;
    std::map<std::string, std::string> headers;
    int bodyFd = -1;
	void parseFirstLine(char *lstart, char *lend);
	void parseHeaderLine(char *lstart, char *lend);
    void addHeader();

public:

    Request();
    ~Request();
    void parseRequestLine(char *lstart, char *lend);
    void setBodyFd();
    void execute();
};

#endif

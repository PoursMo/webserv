#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <string>
#include "../../headers/LocationData.hpp"

class Request
{
    private:
        std::map<std::string, std::string> headers;
        Method method;
    public:
        Request(std::istream http);
        ~Request();
};

#endif

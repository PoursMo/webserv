#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <map>
#include <string>
#include "../../headers/LocationData.hpp"

class Request
{
    private:
        Method method;
        std::string resource;
        std::map<std::string, std::string> headers;
        void handleRequestLine(std::istream http);
        void handleHeaders(std::istream http);
    public:
        Request(std::istream http);
        ~Request();
};

#endif

#ifndef GATEWAYCGI_HPP
#define GATEWAYCGI_HPP

#include <map>
#include <string>
#include "Request.hpp"

class GatewayCGI
{
    private:
        std::map<std::string, std::string> envs;
        std::string script_path;
    public:
        GatewayCGI(const Request& req);
        ~GatewayCGI();
};

#endif

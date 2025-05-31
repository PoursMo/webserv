#ifndef LOGGER_HPP
#define LOGGER_HPP

#include "webserv.hpp"

class Logger
{
private:
    bool isEnabled;
    std::ofstream streamNull;
public:
    Logger();
    ~Logger();
    void setEnabled(bool);
    std::ostream& log();
};

extern Logger logger;

#endif

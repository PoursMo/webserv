#include "Logger.hpp"

Logger::Logger(): isEnabled(false), streamNull("/dev/null", std::ios::out)
{
}

Logger::~Logger()
{
    this->streamNull.close();
}

std::ostream & Logger::log()
{
    if (this->isEnabled)
        return std::cout;
    return this->streamNull;
}

void Logger::setEnabled(bool isEnabled)
{
    this->isEnabled = isEnabled;

}

#include "webserv.hpp"
#include "utils.hpp"


std::string decodeUri(const std::string &src)
{
    std::stringstream decoded;

    decoded << "value: ";
    decoded << str_to_int(src);

    return decoded.str();
}

int main(int ac, char **av)
{

    if (ac != 2)
        return (1);
    std::string uri = av[1];
    std::cout << "Encoded:" << uri << std::endl;
    std::cout << "Decoded:" << decodeUri(uri) << std::endl;

}

#include "uri.hpp"

int main(int ac, char **av)
{
    if (ac != 2)
    {
        std::cout << "Please, provide an uri to decode" << std::endl;
        return 1;
    }
    std::string uri = av[1];
    std::string decoded = decodeUri(uri);
    std::cout << "Encoded: `" << uri << "'" << std::endl;
    std::cout << "Decoded: `" << decoded << "'" << std::endl;
    std::cout << "Encoded: `" << encodeUri(decoded) << "'" << std::endl;
    return 0;
}

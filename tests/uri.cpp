#include "uri.hpp"

int main(int ac, char **av)
{
    if (ac != 2)
    {
        logger.log() << "Please, provide an uri to decode" << std::endl;
        return 1;
    }
    std::string uri = av[1];
    std::string decoded = decodeUri(uri);
    logger.log() << "Encoded: `" << uri << "'" << std::endl;
    logger.log() << "Decoded: `" << decoded << "'" << std::endl;
    logger.log() << "Encoded: `" << encodeUri(decoded) << "'" << std::endl;
    return 0;
}

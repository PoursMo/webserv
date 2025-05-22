#include "uri.hpp"

std::string decodeUri(const std::string &src)
{
    std::stringstream decoded;

    for (std::size_t i = 0; i < src.size(); i++)
    {
        if (src[i] != '%' || i + 3 > src.size())
        {
            decoded << src[i];
            continue ;
        }
        decoded << (char)str_to_int(src.substr(i + 1, 2));
        i += 2;
    }
    return decoded.str();
}

std::string encodeUri(const std::string &src)
{
    static std::string set =":/?#[]@!$&'()*+,;=%";
    std::stringstream encoded;

    for (std::size_t i = 0; i < src.size(); i++)
    {
        if (set.find(src[i]) == std::string::npos)
        {
            encoded << src[i];
            continue ;
        }
        encoded << '%' << int_to_str(src[i], "0123456789ABCDEF");
    }
    return encoded.str();
}

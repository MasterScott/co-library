#include "UrlEncoding.hpp"

#include <sstream>
#include <iomanip>

unsigned char hexToChar(char i)
{
    if (i >= '0' && i <= '9')
        return i - '0';
    if (i >= 'a' && i <= 'f')
        return i - 'a' + 10;
    if (i >= 'A' && i <= 'F')
        return i - 'A' + 10;
    return 0;
}

namespace co
{

std::string urlEncode(const std::string& input)
{
    std::ostringstream stream{};

    for (auto i: input)
    {
        if (std::isalnum(i) || i == '-' || i == '_' || i == '.' || i == '~')
            stream << i;
        else
            stream << '%' << std::uppercase << std::hex << std::setw(2) << unsigned(i) << std::nouppercase;
    }

    return stream.str();
}

std::string urlDecode(const std::string& input)
{
    std::ostringstream stream{};
    std::istringstream inputStream{ input };

    char c;

    while (inputStream.get(c))
    {
        if (c == '+')
            stream << ' ';
        else if (c == '%')
        {
            char result = 0;
            char c1;
            char c2;
            inputStream >> c1 >> c2;
            result |= (hexToChar(c1) << 4) | hexToChar(c2);
            stream << result;
        }
        else
            stream << c;
    }

    return stream.str();
}

}
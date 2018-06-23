#include "HttpRequest.hpp"
#include "UrlEncoding.hpp"

#include <sstream>

#include <stdio.h>
#include <cstdlib>
#include <unistd.h>
#include <cstring> 
#include <sys/socket.h>
#include <netinet/in.h> 
#include <netdb.h>
#include <fcntl.h>

namespace co
{

HttpResponse::HttpResponse(const std::vector<char>& data)
{
    std::ostringstream message{};

    for (size_t i = 0; i < data.size(); ++i)
    {
        if (data[i] == '\r' && (i < data.size() - 1) && data[i + 1] == '\n')
        {
            // \r\n\r\n, body begins
            if (message.str().empty())
            {
                body = std::string(data.begin() + i + 2, data.end());
                return;
            } 
            else 
            {
                if (!got_status)
                {
                    parseStatus(message.str());
                    got_status = true;
                }
                else
                {
                    parseHeader(message.str());
                }
            }
            message.str("");
            ++i;
        }
        else
        {
            message << data[i];
        }
    }
}

std::string HttpResponse::getHeader(const std::string& key) const
{
    std::string key_lower = key;
    for (auto& i: key_lower)
        i = std::tolower(i);
    if (headers.find(key_lower) == headers.end())
        return "";
    return headers.at(key_lower);
}

int HttpResponse::getStatus() const
{
    return status;
}

std::string HttpResponse::getBody() const
{
    return body;
}

void HttpResponse::parseStatus(std::string message)
{
    std::istringstream ss{ message };
    std::string version;
    ss >> version;
    ss >> status;
}

void HttpResponse::parseHeader(std::string message)
{   
    std::string key;
    std::string value;

    auto colon = message.find(':');
    if (colon == std::string::npos)
        throw new std::runtime_error("Malformed header");
    key = message.substr(0, colon);
    auto start = message.find_first_not_of(' ', colon + 1);
    if (start == std::string::npos)
        value = "";
    else
        value = message.substr(start);

    for (char& c: key)
        c = std::tolower(c);

    headers[key] = value;
}

void UrlEncodedBody::add(const std::string& key, const std::string& value)
{
    pairs.push_back(std::make_pair(key, value));
}

UrlEncodedBody::operator std::string() const
{
    std::ostringstream stream{};
    bool first{ true };
    for (const auto& p: pairs)
    {
        if (!first)
            stream << '&';
        else
            first = false;

        stream << urlEncode(p.first) << '=' << urlEncode(p.second);
    }
    return stream.str();
}

HttpRequest::HttpRequest(std::string method, std::string host, int port, std::string path, std::string query)
    : method(method), host(host), port(port), address(path), query(query)
{
    addHeader("Host", port == 80 ? host : host + ":" + std::to_string(port));
}

void HttpRequest::addHeader(const std::string& key, const std::string& value)
{
    headers[key] = value;
}

void HttpRequest::setBody(const std::string& body)
{
    addHeader("Content-Length", std::to_string(body.size()));
    this->body = body;
}

std::vector<char> HttpRequest::serialize() const
{
    std::ostringstream stream{};

    stream << method << ' ' << address;
    if (query.size())
        stream << '?' << query;
    stream << " HTTP/1.0\r\n";
    for (const auto& p: headers)
    {
        stream << p.first << ": " << p.second << "\r\n";
    }
    
    stream << "\r\n" << body;

    std::string result = stream.str();

    return std::vector<char>(result.begin(), result.end());
}

NonBlockingHttpRequest::NonBlockingHttpRequest(const HttpRequest& request)
{
    this->request = request.serialize();
    port = request.port;
    host = request.host;
}

void NonBlockingHttpRequest::send()
{
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        throw new std::runtime_error("Failed to open the socket: " + std::to_string(errno));
    hostent *host = gethostbyname(this->host.c_str());
    if (host == nullptr)
        throw new std::runtime_error("Failed to locate the host: " + std::to_string(errno));
    sockaddr_in server;
    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    memcpy(&server.sin_addr.s_addr, host->h_addr_list[0], host->h_length);

    if (connect(sock, (sockaddr *)&server, sizeof(server)) < 0)
        throw new std::runtime_error("Could not connect to server: " + std::to_string(errno));

    int sent = 0;
    int total = request.size();
    do
    {
        int s = write(sock, request.data() + sent, total - sent);
        if (s < 0)
            throw new std::runtime_error("Could not write to socket: " + std::to_string(errno));
        if (s == 0)
            break;
        sent += s;
    } while (sent < total);

    fcntl(sock, F_SETFL, O_NONBLOCK);
}

bool NonBlockingHttpRequest::update()
{
    if (finished)
        return true;

    while (true)
    {
        size_t f = response.size() - received;
        if (f < 512)
            response.resize(response.size() + 1024);
        f += 512;
        int r = read(sock, response.data() + received, f);
        if (r < 0)
        {
            if (errno == EAGAIN)
                return false;
            else
                throw new std::runtime_error("Could not read from socket: " + std::to_string(errno));
        }
        if (r == 0)
        {
            finished = true;
            return true;
        }
        received += r;
    }
}

HttpResponse NonBlockingHttpRequest::getResponse()
{
    return HttpResponse(std::vector<char>(response.begin(), response.begin() + received));
}

}
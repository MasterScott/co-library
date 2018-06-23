#pragma once

#include <vector>
#include <string>
#include <unordered_map>

namespace co
{

class HttpResponse
{
public:
    HttpResponse(const std::vector<char>& data);

    std::string getHeader(const std::string& key) const;
    int getStatus() const;
    std::string getBody() const;
protected:
    void parseStatus(std::string message);
    void parseHeader(std::string message);

    int status{};
    bool got_status{ false };
    std::unordered_map<std::string, std::string> headers{};
    std::string body{};
};

class UrlEncodedBody
{
public:
    void add(const std::string& key, const std::string& value);
    operator std::string() const;
protected:
    std::vector<std::pair<std::string, std::string>> pairs{};
};

class HttpRequest
{
public:
    HttpRequest(std::string method, std::string host, int port, std::string path, std::string query);

    void addHeader(const std::string& key, const std::string& value);
    void setBody(const std::string& body);

    std::vector<char> serialize() const;

    const std::string method;
    const std::string host;
    const int port;
    const std::string address;
    const std::string query;
protected:
    std::unordered_map<std::string, std::string> headers{};
    std::string body{};
};

class NonBlockingHttpRequest
{
public:
    NonBlockingHttpRequest(const HttpRequest& request);

    void send();
    bool update();

    HttpResponse getResponse();
protected:
    int port;
    std::string host;

    int sock{};
    bool finished{ false };
    int received{ 0 };
    std::vector<char> request{};
    std::vector<char> response{};
};

}
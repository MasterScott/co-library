#pragma once

#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <optional>

#include "HttpRequest.hpp"
#include "json.hpp"

namespace co
{

enum class ApiCallResult
{
    OK,
    BAD_REQUEST,
    UNAUTHORIZED,
    FORBIDDEN,
    NOT_FOUND,
    CONFLICT,
    TOO_MANY_REQUESTS,
    SERVER_ERROR,
    UNKNOWN
};

struct role
{
    role(const nlohmann::json&);

    std::string name;
    std::optional<std::string> display_name;
};

struct software
{
    software(const nlohmann::json&);

    std::string name;
    bool friendly;
};

struct identified_user
{
    identified_user(const nlohmann::json&);

    std::string username;
    bool steamid_verified;
    std::optional<std::string> color;
    std::vector<role> roles;
    std::optional<software> uses_software;
};

struct identified_group
{
    identified_group(const nlohmann::json&);

    std::unordered_map<unsigned, identified_user> users{};
};

struct logged_in_user
{
    logged_in_user(const nlohmann::json&);

    std::string username;
};

class OnlineService
{
public:
    using error_handler_type = std::function<void(std::string)>;

    void setHost(std::string host);
    void setErrorHandler(error_handler_type handler);

    void login(std::string key, std::function<void(ApiCallResult, std::optional<logged_in_user>)> callback);

    void gameStartup(unsigned steamId);
    void userIdentify(const std::vector<unsigned>& steamIdList, std::function<void(ApiCallResult, std::optional<identified_group>)> callback);

    void processPendingCalls();
protected:
    using callback_type = std::function<void(ApiCallResult, HttpResponse&)>;
    using pair_type = std::pair<NonBlockingHttpRequest, callback_type>;
 
    void makeRequest(HttpRequest rq, callback_type callback);
    static ApiCallResult resultFromStatus(int status);
    void error(std::string message);

    std::string host_address{};
    int host_port{};

    bool loggedIn{ false };
    std::string api_key{};

    std::vector<pair_type> pendingCalls{};
    error_handler_type error_handler{ nullptr };
};

}
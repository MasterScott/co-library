#include "OnlineService.hpp"
#include "json.hpp"

namespace co
{

role::role(const nlohmann::json& json)
{
    name = json["name"].get<std::string>();
    if (json["display"].is_null())
        display_name = std::nullopt;
    else
        display_name = json["display"].get<std::string>();
}

software::software(const nlohmann::json& json)
{
    name = json["name"].get<std::string>();
    friendly = json["friendly"].get<bool>();
}

identified_user::identified_user(const nlohmann::json& json)
{
    username = json["username"].get<std::string>();
    steamid_verified = json["verified"].get<bool>();
    if (json["color"].is_null())
        color = std::nullopt;
    else
        color = json["color"].get<std::string>();
    for (auto& e: json["roles"])
    {
        roles.emplace_back(e);
    }
    if (json["software"].is_null())
        uses_software = std::nullopt;
    else
        uses_software = co::software{ json["software"] };
}

identified_group::identified_group(const nlohmann::json& json)
{
    for (auto it = json.begin(); it != json.end(); ++it)
    {
        unsigned steamId = std::stoul(it.key());
        users.emplace(steamId, identified_user{ it.value() });
    }
}

logged_in_user::logged_in_user(const nlohmann::json& json)
{
    username = json["username"].get<std::string>();
}

void OnlineService::makeRequest(HttpRequest rq, callback_type callback)
{
    auto p = std::make_pair(NonBlockingHttpRequest{ rq }, callback);
    p.first.send();
    pendingCalls.push_back(std::move(p));
}

void OnlineService::processPendingCalls()
{
    if (pendingCalls.empty())
        return;

    for (auto it = pendingCalls.begin(); it != pendingCalls.end();)
    {
        if ((*it).first.update())
        {
            auto response = (*it).first.getResponse();
            ApiCallResult result = resultFromStatus(response.getStatus());
            (*it).second(result, response);
            it = pendingCalls.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

void OnlineService::setHost(std::string host)
{
    auto colon = host.find(':');
    if (colon != std::string::npos)
        host_port = std::stoi(host.substr(colon + 1));
    else
        host_port = 80;
    host_address = host.substr(0, colon);
    // DEBUG
    printf("Host = %s, port = %d\n", host_address.c_str(), host_port);
}

void OnlineService::setErrorHandler(error_handler_type handler)
{
    error_handler = handler;
}

void OnlineService::error(std::string message)
{
    if (error_handler)
        error_handler(message);
}

ApiCallResult OnlineService::resultFromStatus(int status)
{
    if (status >= 200 && status < 300)
        return ApiCallResult::OK;
    if (status >= 500)
        return ApiCallResult::SERVER_ERROR;
    if (status == 400)
        return ApiCallResult::BAD_REQUEST;
    if (status == 401)
        return ApiCallResult::UNAUTHORIZED;
    if (status == 403)
        return ApiCallResult::FORBIDDEN;
    if (status == 404)
        return ApiCallResult::NOT_FOUND;
    if (status == 409)
        return ApiCallResult::CONFLICT;
    if (status == 429)
        return ApiCallResult::TOO_MANY_REQUESTS;
    return ApiCallResult::UNKNOWN;
}

void OnlineService::login(std::string key, std::function<void(ApiCallResult, std::optional<logged_in_user>)> callback)
{
    UrlEncodedBody body{};
    body.add("key", key);
    HttpRequest request("GET", host_address, host_port, "/user/me", body);
    api_key = key;
    makeRequest(request, [this, callback](ApiCallResult result, HttpResponse& response) {
        std::optional<logged_in_user> user = std::nullopt;
        if (result == ApiCallResult::OK)
        {
            auto j = nlohmann::json::parse(response.getBody());
            user = logged_in_user{j};
        }
        callback(result, user);
    });
}

void OnlineService::userIdentify(const std::vector<unsigned>& steamIdList, std::function<void(ApiCallResult, std::optional<identified_group>)> callback)
{
    std::ostringstream stream{};
    bool first{ true };
    for (auto& i: steamIdList)
    {
        if (!first)
            stream << ',';
        else
            first = false;
        stream << i;
    }

    UrlEncodedBody query{};
    query.add("key", api_key);
    query.add("ids", stream.str());
    HttpRequest request("GET", host_address, host_port, "/game/identify", query);
    makeRequest(request, [callback](ApiCallResult result, HttpResponse& response) {
        if (result == ApiCallResult::OK)
        {
            auto json = nlohmann::json::parse(response.getBody());
            callback(result, std::make_optional<identified_group>(json));
        }
        else
        {
            callback(result, std::nullopt);
        }
    });
}

}
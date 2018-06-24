#include "co/UrlEncoding.hpp"
#include "co/HttpRequest.hpp"
#include "co/OnlineService.hpp"

#include <iostream>
#include <thread>
#include <chrono>

void urlEncodingTest()
{
    std::cout << "URL ENCODING TEST\n";

    std::cout << co::urlEncode("This is a testing string! '*'") << '\n';
    std::cout << co::urlDecode(co::urlEncode("Testing -.*.-()()()")) << '\n';
    std::cout << co::urlDecode("Testing+string+with+spaces") << '\n';
}

void queryStringTest()
{
    std::cout << "QUERYSTRING TEST\n";

    co::UrlEncodedBody body{};
    body.add("test", "testing");
    //body.add("testing key", "! TESTING VALUE ***");
    body.add("=", "&");
    std::cout << std::string(body) << '\n';
}

void requestSerializeTest()
{
    std::cout << "HTTP REQUEST PARSING TEST\n";    
    co::HttpRequest rq("POST", "localhost", 8000, "/testing url", "E=E");
    rq.addHeader("Content-Type", "application/json");
    rq.setBody("{\"test\":true}");
    auto vec = rq.serialize();
    std::cout << std::string(vec.begin(), vec.end()) << '\n';
}

void httpRequestTest()
{
    std::cout << "HTTP REQUEST/RESPONSE TEST\n";

    co::HttpRequest rq("GET", "nullifiedcat.xyz", 80, "/", "");
    co::NonBlockingHttpRequest nrq(rq);
    try {
        nrq.send();
        while (!nrq.update());
        auto response = nrq.getResponse();
        std::cout << "Got response: " << response.getStatus() << "\n" << response.getBody() << "\n";
        std::cout << "Date: " << response.getHeader("Date") << '\n';
    } catch (std::exception& e)
    {
        std::cout << "Error: " << e.what() << "\n";
    }
}

void onlineServiceTest()
{
    co::OnlineService service{};
    service.setHost("localhost:8000");
    service.setErrorHandler([](std::string error) {
        std::cout << "API Error: " << error << '\n';
    });
    service.login("6b649dee81a4e2ae733eb7cd3be460d5", [](co::ApiCallResult result, std::optional<co::logged_in_user> user) {
        if (user.has_value())
            std::cout << "Logged in as " << user->username << '\n';
        else
            std::cout << "Error logging in\n";
    });
    service.userIdentify({ 123456 }, [](co::ApiCallResult result, std::optional<co::identified_user_group> group) {
        if (!group.has_value())
        {
            std::cout << "Error while identifying users..\n";
            return;
        }
        for (auto& u: group->users)
        {
            std::cout << "Steam3: " << u.first << ": " << u.second.username << "\n";
            std::cout << "Groups: \n";
            for (auto& r: u.second.groups)
            {
                std::cout << '\t' << r.name << " (" << (r.display_name.has_value() ? *r.display_name : "<none>") << ")\n";
            }
            std::cout << "Uses " << (u.second.uses_software.has_value() ? (u.second.uses_software->name) : "unknown software") << "\n";
        }
    });
    service.gameStartup(123456);
    while (true)
    {
        // Somewhere in game loop
        // Non-blocking function
        service.processPendingCalls();

        using namespace std::chrono_literals;
        std::this_thread::sleep_for(1s);
    }
}

int main()
{
    onlineServiceTest();
    return 0;
}
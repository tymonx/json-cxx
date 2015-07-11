#include <json/json.hpp>
#include <json/rpc/client/curl_client.hpp>
#include <json/rpc/client/http_settings.hpp>

#include <iostream>
#include <chrono>

using namespace json;
using json::rpc::time::operator "" _s;
using json::rpc::client::HttpSettings;

static const auto COMMANDS = 2;
static const auto REQUESTS = 500;

int main() {
    HttpSettings settings{};
    settings.set_timeout(3_s);

    rpc::client::CurlClient client{"localhost:6666"};
    client.set_http_settings(settings);
    client.connect();

    auto start = std::chrono::system_clock::now();

    for (unsigned i = 0; i < REQUESTS; ++i) {
        client.method("command1", {
                Pair{"b", "Hej!!!"},
                Pair{"a", true}
            },
            [] (rpc::Client*, const Value& result, const rpc::Error& error) {
                if (error) {
                    std::cout << "Error: " << error.what() << " " << error.get_code() << std::endl;
                }
                else {
                    std::cout << result << std::endl;
                }
            }
        );
        client.method("command2",
                13
            ,
            [] (rpc::Client*, const Value& result, const rpc::Error& error) {
                if (error) {
                    std::cout << "Error: " << error.what() << " " << error.get_code() << std::endl;
                }
                else {
                    std::cout << result << std::endl;
                }
            }
        );
    }

    client.~CurlClient();
    auto stop = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
    std::cout << "Time: " << diff/REQUESTS/COMMANDS << "us" << std::endl;

    return 0;
}

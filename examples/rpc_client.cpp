#include <json/json.hpp>
#include <json/rpc/error.hpp>
#include <json/rpc/client/http_settings.hpp>
#include <json/rpc/client/curl_client.hpp>
#include <json/rpc/client/curl_proactor.hpp>

#include <iostream>
#include <chrono>

using namespace json;
using json::rpc::client::HttpSettings;
using json::rpc::client::CurlProactor;
using json::rpc::Error;

static const auto COMMANDS = 2;
static const auto REQUESTS = 1000;

int main() {
    json::Value value;

    /* max_total_connections < ulimit -n */
    CurlProactor::get_instance().set_max_total_connections(512);

    rpc::client::CurlClient client{"localhost:6666"};
    client.set_id_builder(
        [] (unsigned id) -> std::string {
            return "UUID:" + std::to_string(id);
        }
    );
    client.set_error_to_exception(
        [] (const Error& error) -> std::exception_ptr {
            switch (error.get_code()) {
            case -13:
                return std::make_exception_ptr(
                        std::runtime_error(error.get_message()));
            default:
                return std::make_exception_ptr(error);
            }
        }
    );
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
        client.method("command2", 13,
            [] (rpc::Client*, const Value& result, const rpc::Error& error) {
                if (error) {
                    std::cout << "Error: " << error.what() << " " << error.get_code() << std::endl;
                }
                else {
                    std::cout << result << std::endl;
                }
            }
        );
        /*
        try {
            client.method("commandError", 0, value);
        }
        catch (const std::exception& e) {
            std::cout << "Exception: " << e.what() << std::endl;
        }
        */
    }

    client.~CurlClient();
    auto stop = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::microseconds>(stop - start).count();
    std::cout << "Time: " << diff/REQUESTS/COMMANDS << "us" << std::endl;

    return 0;
}

#include <json/json.hpp>
#include <json/rpc/client/curl_client.hpp>

#include <iostream>
#include <chrono>

using namespace json;
using json::rpc::operator "" _ms;

int main() {
    rpc::client::CurlClient client{{"localhost:6666"}};
    client.set_id_builder([] (unsigned id) {
        return "DUPA-" + std::to_string(id);
    });
    client.connect();

    auto start = std::chrono::system_clock::now();

    for (unsigned i = 0; i < 100; ++i) {
        client.method("command1", {
                Pair{"b", "Hej!!!"},
                Pair{"a", true}
            },
            [] (rpc::Client*, const Value& v, const rpc::Error& error) {
                if (error) {
                    std::cout << "Error: " << error.what() << " " << error.get_code() << std::endl;
                }
                else {
                    std::cout << "OK: " << v << std::endl;
                }
                (void)v;
            }
        );
        client.method("command2", {
                13
            },
            [] (rpc::Client*, const Value& v, const rpc::Error& error) {
                if (error) {
                    std::cout << "Error: " << error.what() << " " << error.get_code() << std::endl;
                }
                else {
                    std::cout << "OK: " << v << std::endl;
                }
                (void)v;
            }
        );
    }

    auto stop = std::chrono::system_clock::now();
    auto diff = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start).count();
    std::cout << "Time: " << diff << "ns" << std::endl;

    return 0;
}

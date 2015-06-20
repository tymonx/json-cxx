#include <json/json.hpp>
#include <json/rpc/client.hpp>
#include <json/rpc/client/http_protocol.hpp>

#include <iostream>
#include <chrono>

using namespace json;
using json::rpc::operator "" _ms;

int main() {
    rpc::client::HttpProtocol http{"localhost:6666"};
    http.set_id_builder([] (unsigned id) {
        return "DUPA-" + std::to_string(id);
    });
    rpc::Client client(http);

    auto start = std::chrono::system_clock::now();

    for (unsigned i = 0; i < 100; ++i) {
    client.method("xxx", 8,
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

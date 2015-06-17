#include <json/json.hpp>
#include <json/rpc/client.hpp>
#include <json/rpc/time.hpp>
#include <json/rpc/client/http_protocol.hpp>

#include <iostream>

using namespace json;
using json::rpc::operator "" _ms;

int main() {
    Value value;
    rpc::client::HttpProtocol http{"localhost:6666"};
    http.set_timeout(0_ms);
    rpc::Client client(http);

    client.method("sexy", 5, value);
    auto vc = client.method("doopy", 7);
    vc.get();

    for (unsigned i = 0; i < 100000; ++i) {
    client.method("xxx", 8,
        [] (const Value& v, const rpc::Error& error) {
            if (error) {
                std::cout << "Error: " << error.what() << " " << int(error.get_code()) << std::endl;
            }
            (void)v;
        }
    );
    }

    return 0;
}

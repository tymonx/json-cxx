#include <json/json.hpp>
#include <json/rpc/client.hpp>
#include <json/rpc/client/http_protocol.hpp>

#include <iostream>

using namespace json;

int main() {
    Value value;
    rpc::client::HttpProtocol http{"localhost:6666"};
    rpc::Client client(http);

    client.method("sexy", 5, value);
    auto vc = client.method("doopy", 7);
    client.method("xxx", 8,
        [] (const Value& v, const rpc::Error& error) {
            if (error) {
                std::cout << "Error: " << error.what() << " " <<error.get_code() << std::endl;
            }
            (void)v;
        }
    );
    vc.get();

    return 0;
}

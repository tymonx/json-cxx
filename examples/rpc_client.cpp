#include <json/json.hpp>
#include <json/rpc/client.hpp>
#include <json/rpc/client/http_protocol.hpp>

#include <iostream>

using namespace json;

int main() {
    Value value;
    rpc::client::HttpProtocol http{};
    rpc::Client client(http);

    client.method("sexy", 5, value);
    value = client.method("doopy", 7).get();
    client.method("xxx", 8,
        [] (const Value& v, const rpc::Error& error) {
            if (error) {
                std::cout << "Error: " << error.what() << std::endl;
            }
            (void)v;
        }
    );

    return 0;
}

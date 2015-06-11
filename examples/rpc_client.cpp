#include <json/json.hpp>
#include <json/rpc/client.hpp>
#include <json/rpc/client/protocol/ipv4.hpp>

using namespace json;

int main() {
    Value value;
    rpc::client::protocol::IPv4 ipv4;
    rpc::Client client(ipv4);

    client.method("sexy", 5, value);
    value = client.method("doopy", 7);
    client.method("xxx", 8,
        [] (const Value& v) {
            (void)v;
        }
    );

    return 0;
}

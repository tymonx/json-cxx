#include <iostream>

#include <json/rpc/server/microhttpd_server.hpp>

using std::cout;
using std::endl;
using json::Pair;
using json::Value;
using json::rpc::server::MicrohttpdServer;

int main(int argc, char* argv[]) {
    MicrohttpdServer::Port port = 6666;

    if (argc > 1) {
        port = MicrohttpdServer::Port(std::stoi(argv[1]));
    }

    MicrohttpdServer server(port);

    server.add_command(
        "xxx", {
            Pair{"a", Value::Type::BOOLEAN},
            Pair{"b", Value::Type::STRING}
        },
        [] (const Value& request, Value& response, const Value& id) {
            cout << "Request: " << request << " id: " << id << endl;
            response = "Dupa!!!";
        }
    );

    server.start();
    cout << "Daemon started..." << endl;
    std::getchar();
    server.stop();
    cout << "Daemon stopped" << endl;

    return 0;
}

#include <iostream>

#include <json/rpc/server/microhttpd_server.hpp>

using std::cout;
using std::endl;
using json::Pair;
using json::Value;
using json::rpc::server::MicrohttpdServer;
using json::rpc::Server;

int main(int argc, char* argv[]) {
    MicrohttpdServer::Port port = 6666;

    if (argc > 1) {
        port = MicrohttpdServer::Port(std::stoi(argv[1]));
    }

    MicrohttpdServer server(port);

    server.add_command(
        "command1",
        [] (const Value& params, Value& result, const Value& id) {
            cout << params << ":" << id << endl;
            result = "Response from command1!!!";
        }
    );
    server.add_command(
        "command2",
        [] (const Value& params, Value& result, const Value& id) {
            cout << params << ":" << id << endl;
            result = "Response from command2!!!";
        }
    );

    server.start();
    cout << "Daemon started..." << endl;
    std::getchar();
    server.stop();
    cout << "Daemon stopped" << endl;

    return 0;
}

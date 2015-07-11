#include <iostream>

#include <json/rpc/server/microhttpd_server.hpp>
#include <json/rpc/server/http_settings.hpp>
#include <atomic>

using std::cout;
using std::endl;
using json::Pair;
using json::Value;
using json::rpc::server::MicrohttpdServer;
using json::rpc::server::HttpSettings;
using json::rpc::Server;

static volatile std::atomic_uint counter{0};

int main(int argc, char* argv[]) {
    MicrohttpdServer::Port port = 6666;

    if (argc > 1) {
        port = MicrohttpdServer::Port(std::stoi(argv[1]));
    }

    MicrohttpdServer server(port);
    HttpSettings settings{};


    server.add_command(
        "command1",
        [] (const Value& params, Value& result, const Value& id) {
            ++counter;
            cout << params << ":" << id << endl;
            result = "Response from command1!!!";
        }
    );
    server.add_command(
        "command2",
        [] (const Value& params, Value& result, const Value& id) {
            ++counter;
            cout << params << ":" << id << endl;
            result = "Response from command2!!!";
        }
    );

    server.start();
    cout << "Daemon started..." << endl;
    std::getchar();
    server.stop();
    cout << "Daemon stopped. Messages: " << counter << endl;

    return 0;
}

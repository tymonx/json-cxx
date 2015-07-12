#include <iostream>

#include <json/rpc/error.hpp>
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
using json::rpc::Error;
using json::rpc::time::operator "" _s;

static volatile std::atomic_uint counter{0};

int main(int argc, char* argv[]) {
    MicrohttpdServer::Port port = 6666;
    HttpSettings settings{};
    settings.set_timeout(3_s);

    if (argc > 1) {
        port = MicrohttpdServer::Port(std::stoi(argv[1]));
    }

    MicrohttpdServer server(port);
    server.set_settings(settings);
    server.set_method_handler(
        [] (const Server::MethodId& method, const Value& params, Value& result,
            const Value& id) {
            try {
                method(params, result, id);
            }
            catch (const std::runtime_error& e) {
                throw Error{-13, e.what()};
            }
            catch (const std::exception& e) {
                throw Error{-2, e.what()};
            }
            catch (...) {
                throw Error{-3, "Error"};
            }
        }
    );

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

    server.add_command(
        "commandError",
        [] (const Value& params, Value&, const Value& id) {
            ++counter;
            cout << params << ":" << id << endl;
            throw std::runtime_error("Runtime error");
        }
    );

    server.start();
    cout << "Daemon started..." << endl;
    std::getchar();
    server.stop();
    cout << "Daemon stopped. Messages: " << counter << endl;

    return 0;
}

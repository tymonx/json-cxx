#include <chrono>
#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>
#include "json/json.hpp"

using namespace std;

static const uint32_t ntimes = 10;

int main(int argc, char** argv) {

    if (argc < 2)
        return -1;

    ifstream inputfile(argv[1]);
    std::string to_parse;

    inputfile.seekg(0, std::ios::end);
    to_parse.reserve(size_t(inputfile.tellg()));
    inputfile.seekg(0, std::ios::beg);

    to_parse.assign((std::istreambuf_iterator<char>(inputfile)),
                                 std::istreambuf_iterator<char>());


    cout << "Start parsing" << endl;

    chrono::steady_clock::time_point start_time = chrono::steady_clock::now();
    for (size_t i = 0; i < ntimes; ++i) {
            Json::Value parsed_object;
            Json::Reader parser;
            (void)parser.parse(to_parse, parsed_object);
        }
    chrono::steady_clock::time_point end_time = chrono::steady_clock::now();
    chrono::microseconds us = chrono::duration_cast<chrono::microseconds>(
                                           end_time - start_time);
    cout << "[+] Finished successfully with an average of: " << (us.count() / ntimes) << " us\n" << endl;

    cout << "Start parsing" << endl;

    start_time = chrono::steady_clock::now();
    for (size_t i = 0; i < ntimes; ++i) {
            json::Value value;
            json::Deserializer(to_parse) >> value;
        }
    end_time = chrono::steady_clock::now();
    us = chrono::duration_cast<chrono::microseconds>(
                                           end_time - start_time);

    cout << "[+] Finished successfully with an average of: " << (us.count() / ntimes) << " us\n" << endl;
}

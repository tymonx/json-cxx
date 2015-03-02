#include "json/json.hpp"
#include "jsoncpp/json/json.h"

#include <chrono>
#include <iostream>

using std::cout;
using std::endl;

using namespace json;

static constexpr auto TEST_COUNT = 1000000;

int main() {
    Value value;

    auto start = std::chrono::steady_clock::now();

    for (auto count = 0; count < TEST_COUNT; ++count) {
        R"({"key":"test"})" >> value;

        if (value["key"] != "test") {
            return 0;
        }
    }

    auto stop = std::chrono::steady_clock::now();
    auto diff = stop - start;

    cout << std::chrono::duration<double, std::nano>(diff).count()/TEST_COUNT << " ns" << endl;

    Json::Value root;
    Json::Reader reader;

    start = std::chrono::steady_clock::now();

    for (auto count = 0; count < TEST_COUNT; ++count) {
        reader.parse(R"({"key":"test"})", root);

        if (root["key"] != "test") {
            return 0;
        }
    }

    stop = std::chrono::steady_clock::now();
    diff = stop - start;

    cout << std::chrono::duration<double, std::nano>(diff).count()/TEST_COUNT << " ns" << endl;
}

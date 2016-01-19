#include <json/json.hpp>

using namespace json;

int main() {
    json::Value value;
    R"(-0.00000000001e+20)" >> value;
}

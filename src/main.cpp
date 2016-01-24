
#include <json/json.hpp>
#include <json/allocator/pool.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <limits>
#include <array>
#include <algorithm>
#include <iomanip>

using namespace json;
using namespace std;

static std::array<char, 8*128> g_pool{{}};

static void dump() {
    for (size_t i = 0; i < g_pool.size();) {
        for (size_t j = 0; j < 32 && i < g_pool.size(); ++i, ++j) {
            std::cout << std::hex << setw(2) << setfill('0')
                << unsigned(static_cast<unsigned char>(g_pool[i])) << " ";
        }
        cout << endl;
    }
}

int main() {
# if 1
    std::cout << std::hex << setw(2) << setfill('0');

    json::allocator::Pool pool(g_pool.data(), g_pool.size());

    json::Value value(&pool);
    //json::Value value;
    //R"(-0.00000000001e+20)" >> value;
/*
    cout << "Is number: " << value.is_number() << endl;
    cout << "Is double: " << value.is_double() << endl;
    cout << "Is uint: "   << value.is_uint() << endl;
    cout << "Is int: "    << value.is_int() << endl;
    cout << "Int: "       << Int(value) << endl;
    cout << "Uint: "      << Uint(value) << endl;
    cout << "Double: "    << Double(value) << endl;

    R"( " 01asafaf12.oe \u2708 \u263A \uD83D\uDE02"  )" >> value;
*/
    Object object(&pool);
    //Object object;
    dump();
    object["test1"] = 0;
    dump();
    cout << object[0].key << endl;
    object["test2"] = 1;
    dump();
    cout << object[0].key << endl;
    cout << object[1].key << endl;
    object["test3"] = 2;
    dump();
    cout << object[0].key << endl;
    cout << object[1].key << endl;
    //cout << object[2].key << endl;
    std::for_each(object.crbegin(),object.crend(),
        [] (const Pair& pair) {
            cout << pair.key.size() << std::endl;
            cout << int(pair.key[5]) << std::endl;
            cout << pair.key.cbegin() << std::endl;
        }
    );
    object.~Object();
/*
    cout << "Is string: " << value.is_string() << endl;
    cout << "String: '" << String(value) << "'" << endl;

    cout << "A>" << endl;
    R"( true  )" >> value;
    cout << "<A" << endl;

    cout << "Is bool: " << value.is_bool() << endl;
    cout << "Bool: '" << Bool(value) << "'" << endl;

    R"( false  )" >> value;

    cout << "Is bool: " << value.is_bool() << endl;
    cout << "Bool: '" << Bool(value) << "'" << endl;

    R"( null  )" >> value;

    cout << "Is null: " << value.is_null() << endl;

    cout << "B>" << endl;
    R"( {"abc": 5, "bvc": 7}  )" >> value;
    cout << "<B" << endl;

    cout << "Is object: " << value.is_object() << endl;

    R"([0, 2, 4])" >> value;
    cout << "C>" << endl;
    R"( [0, 1, 2, {"aa": 2, "123": [0, 1]}]  )" >> value;
    cout << "<C" << endl;
*/
    cout << "Is array: " << value.is_array() << endl;
    cout << endl << dec;
    cout << "Null size: " << sizeof(json::Null) << endl;
    cout << "Value size: " << sizeof(json::Value) << endl;
    cout << "Array size: " << sizeof(json::Array) << endl;
    cout << "Object size: " << sizeof(json::Object) << endl;
    cout << "Object size: " << sizeof(std::vector<Object>) << endl;
    cout << "String size: " << sizeof(json::String) << endl;
    cout << "String size: " << sizeof(std::string) << endl;
    cout << "Number size: " << sizeof(json::Number) << endl;
    cout << "Pair size: " << sizeof(json::Pair) << endl;
    
#endif
}



#include <json/json.hpp>
#include <json/allocator/pool.hpp>

#include <cstdint>
#include <iostream>
#include <string>
#include <limits>
#include <array>

using namespace json;
using std::cout;
using std::endl;
using std::numeric_limits;


static std::array<char, 1 * 1024> g_pool{};

//int main() { }

# if 1
int main() {
    json::allocator::Pool pool(g_pool.data(), g_pool.size());

    json::Value value(&pool);
    R"(-0.00000000001e+20)" >> value;

    cout << "Is number: " << value.is_number() << endl;
    cout << "Is double: " << value.is_double() << endl;
    cout << "Is uint: "   << value.is_uint() << endl;
    cout << "Is int: "    << value.is_int() << endl;
    cout << "Int: "       << Int(value) << endl;
    cout << "Uint: "      << Uint(value) << endl;
    cout << "Double: "    << Double(value) << endl;

    R"( " 01asafaf12.oe \u2708 \u263A \uD83D\uDE02"  )" >> value;

#if 1
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

    cout << "Is array: " << value.is_array() << endl;
#endif
    cout << endl;
    cout << "Null size: " << sizeof(json::Null) << endl;
    cout << "Value size: " << sizeof(json::Value) << endl;
    cout << "Array size: " << sizeof(json::Array) << endl;
    cout << "Object size: " << sizeof(json::Object) << endl;
    cout << "String size: " << sizeof(json::String) << endl;
    cout << "String size: " << sizeof(std::string) << endl;
    cout << "Number size: " << sizeof(json::Number) << endl;
    cout << "Pair size: " << sizeof(json::Pair) << endl;

    cout << "-----------------------------------------------" << endl;
    cout << "" << numeric_limits<ptrdiff_t>::max() << endl;
    cout << "" << numeric_limits<ptrdiff_t>::min() << endl;
    cout << "" << numeric_limits<uint64_t>::digits << endl;
    cout << "" << numeric_limits<uint64_t>::digits10 << endl;
    cout << "" << numeric_limits<uint64_t>::max_digits10 << endl;

    cout << "-----------------------------------------------" << endl;
    cout << "" << numeric_limits<int64_t>::max() << endl;
    cout << "" << numeric_limits<int64_t>::min() << endl;
    cout << "" << numeric_limits<int64_t>::digits << endl;
    cout << "" << numeric_limits<int64_t>::digits10 << endl;
    cout << "" << numeric_limits<int64_t>::max_digits10 << endl;

    cout << "-----------------------------------------------" << endl;
    cout << "" << numeric_limits<uint32_t>::max() << endl;
    cout << "" << numeric_limits<uint32_t>::min() << endl;
    cout << "" << numeric_limits<uint32_t>::digits << endl;
    cout << "" << numeric_limits<uint32_t>::digits10 << endl;
    cout << "" << numeric_limits<uint32_t>::max_digits10 << endl;

    cout << "-----------------------------------------------" << endl;
    cout << "" << numeric_limits<int32_t>::max() << endl;
    cout << "" << numeric_limits<int32_t>::min() << endl;
    cout << "" << numeric_limits<int32_t>::digits << endl;
    cout << "" << numeric_limits<int32_t>::digits10 << endl;
    cout << "" << numeric_limits<int32_t>::max_digits10 << endl;

    cout << "-----------------------------------------------" << endl;
    cout << "" << numeric_limits<uint16_t>::max() << endl;
    cout << "" << numeric_limits<uint16_t>::min() << endl;
    cout << "" << numeric_limits<uint16_t>::digits << endl;
    cout << "" << numeric_limits<uint16_t>::digits10 << endl;
    cout << "" << numeric_limits<uint16_t>::max_digits10 << endl;

    cout << "-----------------------------------------------" << endl;
    cout << "" << numeric_limits<int16_t>::max() << endl;
    cout << "" << numeric_limits<int16_t>::min() << endl;
    cout << "" << numeric_limits<int16_t>::digits << endl;
    cout << "" << numeric_limits<int16_t>::digits10 << endl;
    cout << "" << numeric_limits<int16_t>::max_digits10 << endl;

    cout << "-----------------------------------------------" << endl;
    cout << "" << int(numeric_limits<uint8_t>::max()) << endl;
    cout << "" << numeric_limits<uint8_t>::digits << endl;
    cout << "" << numeric_limits<uint8_t>::digits10 << endl;
    cout << "" << numeric_limits<uint8_t>::max_digits10 << endl;

    cout << "" << int(numeric_limits<int8_t>::max()) << endl;
    cout << "" << numeric_limits<int8_t>::digits << endl;
    cout << "" << numeric_limits<int8_t>::digits10 << endl;
    cout << "" << numeric_limits<int8_t>::max_digits10 << endl;


}
#endif

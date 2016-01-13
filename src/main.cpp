
#include <json/json.hpp>

#include <iostream>

using namespace std;
using namespace json;

int main() {
    cout << "Null size: " << sizeof(Null) << endl;
    cout << "Value size: " << sizeof(Value) << endl;
    cout << "Array size: " << sizeof(Array) << endl;
    cout << "Object size: " << sizeof(Object) << endl;
    cout << "String size: " << sizeof(String) << endl;
    cout << "Number size: " << sizeof(Number) << endl;
}

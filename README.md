# JSON-C++
JSON library written in C++11

## Build

    mkdir build
    cd build
    cmake ..
    make

## Build with code coverage

    mkdir build
    cd build
    cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Coverage -DCODE_COVERAGE=ON ..
    make code_coverage
    <HTML_BROWSER> coverage-html/index.html

## Build with memory check

    mkdir build
    cd build
    cmake -DMEMORY_CHECK=ON ..
    make memory_check

## Build with code coverage and memory check support

    mkdir build
    cd build
    cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Coverage -DCODE_COVERAGE=ON -DMEMORY_CHECK=ON ..
    make code_coverage memory_check
    <HTML_BROWSER> coverage-html/index.html

## Examples

    cd build
    bin/example

## Tests

    cd build
    bin/tests_runner

## Test samples

    wget https://raw.githubusercontent.com/sanSS/json-bechmarks/master/data/small-dict.json
    wget https://raw.githubusercontent.com/sanSS/json-bechmarks/master/data/medium-dict.json
    wget https://raw.githubusercontent.com/sanSS/json-bechmarks/master/data/large-dict.json

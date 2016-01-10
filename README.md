# JSON-C++
JSON library written in C++11

## Prepare build workspace

    mkdir build
    cd build

## Build

    cmake ..
    make

## Build with code coverage support

    cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Coverage -DCODE_COVERAGE=ON ..
    make code_coverage
    <HTML_BROWSER> ./coverage-html/index.html

## Build with memory check support

    cmake -DMEMORY_CHECK=ON ..
    make memory_check

## Build with code coverage and memory check support

    cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_BUILD_TYPE=Coverage -DCODE_COVERAGE=ON -DMEMORY_CHECK=ON ..
    make code_coverage memory_check
    <HTML_BROWSER> ./coverage-html/index.html

## Examples

    ./bin/example

## Tests

    ./bin/tests_runner

## Install headers and library

    cmake -DCMAKE_INSTALL_PREFIX=<dir> ..
    make install

## Test samples

    wget https://raw.githubusercontent.com/sanSS/json-bechmarks/master/data/small-dict.json
    wget https://raw.githubusercontent.com/sanSS/json-bechmarks/master/data/medium-dict.json
    wget https://raw.githubusercontent.com/sanSS/json-bechmarks/master/data/large-dict.json

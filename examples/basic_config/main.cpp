#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <memory>

#include <storyteller/storyteller.hpp>

// process config file
    // --process -f config.json
    // --process --file config.json
// simulate default example
    // --simulate --example
// simulate particle from database
    // --simulate -f config.json --particle -s 0
    // --simulate --file config.json --particle --serial 0
// simulate multiple particles from database starting with specified serial
    // --simulate -f config.json --particle -s 0 -b 2
    // --simulate --file config.json --particle --serial 0 --batch 2

int main(int argc, char* argv[]) {
    Storyteller storyteller(argc, argv);
    return storyteller.run();

    return 0;
}
#include <iostream>

#include "simulator.hpp"

int main()
{
    // init simulation parameterization (read from sqlite db?)
    // construct community
    Simulator sim;
    sim.init();
    // simulate
    sim.simulate();
    // return desired metrics
    sim.results();
    return 0;
}
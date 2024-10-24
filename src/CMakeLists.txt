cmake_minimum_required(VERSION 3.22)

set(HEADER_LIST
    simulator.hpp
    community.hpp
    parameters.hpp
    person.hpp
    utility.hpp
)

add_library(core
    simulator.cpp
    community.cpp
    parameters.cpp
    person.cpp
    utility.cpp
    ${HEADER_LIST}
)
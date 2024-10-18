cmake_minimum_required(VERSION 3.22)

project(basic_contin_suscep_model
    VERSION 0.1
    LANGUAGES CXX)

set(HEADER_LIST
    simulator.hpp
    community.hpp
    parameters.hpp
    person.hpp
)

add_library(core
    simulator.cpp
    community.cpp
    parameters.cpp
    person.cpp
    ${HEADER_LIST}
)

add_executable(sim main.cpp)

target_link_libraries(sim PRIVATE core)

find_package(GSL REQUIRED)
target_link_libraries(sim PRIVATE GSL::gsl GSL::gslcblas)

find_package(SQLite3 REQUIRED)
target_link_libraries(sim PRIVATE SQLite::SQLite3)
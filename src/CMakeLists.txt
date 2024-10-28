cmake_minimum_required(VERSION 3.22)

file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/simulator/*.hpp")

add_library(
    storyteller
    simulator.cpp
    community.cpp
    parameters.cpp
    person.cpp
    utility.cpp
    ledger.cpp
    ${HEADER_LIST}
)

target_include_directories(storyteller PUBLIC ${CMAKE_SOURCE_DIR}/include)

find_package(GSL REQUIRED)
target_link_libraries(storyteller PRIVATE GSL::gsl GSL::gslcblas)

target_link_libraries(storyteller PRIVATE SQLiteCpp)

target_link_libraries(storyteller PRIVATE nlohmann_json::nlohmann_json)
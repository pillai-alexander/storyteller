cmake_minimum_required(VERSION 3.22)

file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/include/simulator/*.hpp")

add_library(core
    simulator.cpp
    community.cpp
    parameters.cpp
    person.cpp
    utility.cpp
    ledger.cpp
    ${HEADER_LIST}
)

target_include_directories(core PUBLIC ${CMAKE_SOURCE_DIR}/include)

find_package(GSL REQUIRED)
target_link_libraries(core PRIVATE GSL::gsl GSL::gslcblas)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/SQLiteCpp sqlitecpp_build)
target_link_libraries(core PRIVATE SQLiteCpp)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/json json_build)
target_link_libraries(core PRIVATE nlohmann_json::nlohmann_json)
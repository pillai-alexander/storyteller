cmake_minimum_required(VERSION 3.22)

set(HEADER_LIST
    simulator.hpp
    community.hpp
    parameters.hpp
    person.hpp
    utility.hpp
    ledger.hpp
)

add_library(core
    simulator.cpp
    community.cpp
    parameters.cpp
    person.cpp
    utility.cpp
    ledger.cpp
    ${HEADER_LIST}
)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/SQLiteCpp sqlitecpp_build)
target_link_libraries(core PRIVATE SQLiteCpp)

add_subdirectory(${CMAKE_SOURCE_DIR}/lib/json json_build)
target_link_libraries(core PRIVATE nlohmann_json::nlohmann_json)
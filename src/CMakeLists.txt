cmake_minimum_required(VERSION 3.22)

file(GLOB HEADER_LIST CONFIGURE_DEPENDS "${Storyteller_SOURCE_DIR}/include/simulator/*.hpp")

add_library(
    storyteller
    storyteller.cpp
    simulator.cpp
    database_handler.cpp
    tome.cpp
    parameters.cpp
    utility.cpp
    ledger.cpp
    community.cpp
    person.cpp
    ${HEADER_LIST}
)

target_include_directories(storyteller PUBLIC ${Storyteller_SOURCE_DIR}/include)

find_package(GSL REQUIRED)
target_link_libraries(storyteller PRIVATE GSL::gsl GSL::gslcblas)

target_link_libraries(storyteller PRIVATE ${LUA_LIB})
target_include_directories(storyteller PRIVATE ${LUA_INCLUDE_DIR})

target_link_libraries(storyteller PRIVATE sol2)
target_include_directories(storyteller PRIVATE sol2)
add_dependencies(sol2 lua)

target_link_libraries(storyteller PRIVATE SQLiteCpp)


target_link_libraries(storyteller PRIVATE argh)
target_include_directories(storyteller PUBLIC ${argh_SOURCE_DIR})

# IDEs should put the headers in a nice place
source_group(
  TREE "${Storyteller_SOURCE_DIR}/include"
  PREFIX "Header Files"
  FILES ${HEADER_LIST}
)
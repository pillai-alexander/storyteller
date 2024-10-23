cmake_minimum_required(VERSION 3.22)

project(basic_contin_suscep_model
    VERSION 0.1
    LANGUAGES CXX)

add_subdirectory(src)

add_executable(sim main.cpp)

target_include_directories(sim PRIVATE src)

target_link_libraries(sim PRIVATE core)

find_package(GSL REQUIRED)
target_link_libraries(sim PRIVATE GSL::gsl GSL::gslcblas)

# add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/SQLiteCpp)
# target_link_libraries(sim PRIVATE SQLiteCpp)

# add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/lib/json)
# target_link_libraries(sim PRIVATE nlohmann_json::nlohmann_json)
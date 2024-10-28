cmake_minimum_required(VERSION 3.22)

project(
    Storyteller
    VERSION 0.1
    LANGUAGES CXX
)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message("-- Building examples")
    add_subdirectory(examples)
endif()

include(FetchContent)
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG 9cca280a4d0ccf0c08f47a99aa71d1b0e52f8d03 #v3.11.3
)

FetchContent_Declare(
    SQLiteCpp
    GIT_REPOSITORY https://github.com/SRombauts/SQLiteCpp.git
    GIT_TAG 08aa70a45ea52abcd8ee6b5d1ab1542140b3c7f5 #v3.3.2
)

FetchContent_MakeAvailable(nlohmann_json SQLiteCpp)

add_subdirectory(src)
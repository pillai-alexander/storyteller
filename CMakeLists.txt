cmake_minimum_required(VERSION 3.22)

project(
    Storyteller
    VERSION 0.1
    LANGUAGES CXX
)

include(ExternalProject)

ExternalProject_Add(
    lua
    URL "https://www.lua.org/ftp/lua-5.4.7.tar.gz"
    CONFIGURE_COMMAND ""
    BUILD_COMMAND make all
    BUILD_IN_SOURCE true
    INSTALL_COMMAND make local
)

ExternalProject_Get_Property(lua BINARY_DIR)
set(LUA_INCLUDE_DIR ${BINARY_DIR}/src)
set(LUA_LIB ${BINARY_DIR}/src/liblua.a)

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

FetchContent_Declare(
  sol2
  GIT_REPOSITORY https://github.com/ThePhD/sol2.git
  GIT_TAG eba86625b707e3c8c99bbfc4624e51f42dc9e561 #v3.3.0
)

FetchContent_Declare(
  argh
  GIT_REPOSITORY https://github.com/adishavit/argh.git
  GIT_TAG d17062c870b5919f6d1eae7fe12879869a893b32 #v1.3.2
)

FetchContent_MakeAvailable(nlohmann_json SQLiteCpp sol2 argh)

option(BUILD_DOCS "Build Doxygen documentation" OFF)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message("-- Building examples")
    add_subdirectory(examples)

    if(BUILD_DOCS)
        message("-- Building doxy documentation")
        add_subdirectory(docs)
    endif()
endif()

add_subdirectory(src)
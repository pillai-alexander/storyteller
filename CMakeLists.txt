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

add_subdirectory(src)
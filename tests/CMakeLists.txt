cmake_minimum_required(VERSION 3.19)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG b514bdc898e2951020cbdca1304b75f5950d1f59 #v1.15.2
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(hello_test hello_test.cpp)
target_link_libraries(hello_test GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(hello_test)
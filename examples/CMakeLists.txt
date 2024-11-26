cmake_minimum_required(VERSION 3.19...3.22)

add_executable(example main.cpp)

target_link_libraries(example PRIVATE storyteller)

set_target_properties(example
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
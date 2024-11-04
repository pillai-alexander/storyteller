cmake_minimum_required(VERSION 3.22)

add_executable(basic_config main.cpp)

target_link_libraries(basic_config PRIVATE storyteller)

# target_link_libraries(basic_config PRIVATE argh)
# target_include_directories(basic_config PRIVATE argh)

set_target_properties(basic_config
    PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
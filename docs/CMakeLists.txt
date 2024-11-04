cmake_minimum_required(VERSION 3.22)

find_package(Doxygen REQUIRED doxygen)
if(DOXYGEN_FOUND)
    message("-- Building Doxygen configuration")
    set(DOXY_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXY_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    CONFIGURE_FILE(${DOXY_IN} ${DOXY_OUT} @ONLY)

    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXY_OUT}
        WORKING_DIRECTORY ${Storyteller_SOURCE_DIR}
        VERBATIM
    )
endif()
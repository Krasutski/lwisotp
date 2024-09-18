cmake_minimum_required(VERSION 3.10)

###
# Project definition
###
project(isotp LANGUAGES C)

###
# Get all include directories
###
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/
)


###
# Compile isotp as Shared Lib
###
add_library(isotp STATIC
            isotp.c)

    set(COMPILE_OPTIONS_C
    -fprofile-arcs  #generate coverage information
    -ftest-coverage  #generate coverage information
    -g
    -m32
    -pedantic-errors
    -Wall
    # -Werror
    -Wextra
    -Wno-long-long
    -Wno-pedantic
    -Wshadow
    -Wswitch-enum
    # -Wconversion
    # -Wsign-conversion
    # -Wswitch-default
)


set(COMPILE_OPTIONS_CXX
    ${COMPILE_OPTIONS_C}
    -Woverloaded-virtual
)


target_compile_options(${PROJECT_NAME}
    PUBLIC
        $<$<COMPILE_LANGUAGE:C>:${COMPILE_OPTIONS_C}>
        $<$<COMPILE_LANGUAGE:CXX>:${COMPILE_OPTIONS_CXX}>
)

target_link_options(${PROJECT_NAME}
    PUBLIC
        ${COMPILE_OPTIONS_CXX}
)

add_subdirectory(tests)

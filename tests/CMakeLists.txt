project(test_runner)

file(GLOB TEST_SOURCES
     "src/*_test.cpp"
)

if(DEFINED ENV{CPPUTEST_HOME})
    message(STATUS "CPPUTEST_HOME:" $ENV{CPPUTEST_HOME})
else()
    message(FATAL_ERROR "CPPUTEST_HOME not defined" )
endif()

set(CPPUTEST_HOME $ENV{CPPUTEST_HOME})

set(COMPILE_OPTIONS_C
    -fprofile-arcs  #generate coverage informarion
    -ftest-coverage  #generate coverage informarion
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

#debug purpose, run only one test
# set(TEST_SOURCES src/dummy_test.cpp)

add_executable(${PROJECT_NAME}
    ${TEST_SOURCES}
    ${CMAKE_CURRENT_SOURCE_DIR}/test_runner.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/mock/isotp_user_send_can_mock.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fake/isotp_user_get_ms_fake.cpp
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

target_include_directories(${PROJECT_NAME}
    PUBLIC
        .
        ${CPPUTEST_HOME}/include
)

target_link_directories(${PROJECT_NAME}
    PRIVATE
        ${CPPUTEST_HOME}/lib
)

target_precompile_headers(${PROJECT_NAME}
    PRIVATE
        ${CPPUTEST_HOME}/include/CppUTest/MemoryLeakDetectorMallocMacros.h
        ${CPPUTEST_HOME}/include/CppUTest/MemoryLeakDetectorNewMacros.h
)

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        CppUTest
        CppUTestExt
        isotp
)

SET(COVERAGE_PATH ${CMAKE_BINARY_DIR}/coverage_report)

add_custom_target(generate-coverage DEPENDS test_runner
    COMMAND rm -rf ${COVERAGE_PATH}
    COMMAND mkdir ${COVERAGE_PATH}
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/test_runner
    COMMAND lcov --rc lcov_branch_coverage=1 --capture --directory ${CMAKE_CURRENT_BINARY_DIR}/../CMakeFiles/isotp.dir -o ${COVERAGE_PATH}/tests.info
    # COMMAND lcov --rc lcov_branch_coverage=1 --remove ${COVERAGE_PATH}/tests.info '${CMAKE_CURRENT_SOURCE_DIR}/*' '/opt/cpputest/*' '/usr/include/*' -o ${COVERAGE_PATH}/tests.info
    COMMAND genhtml ${COVERAGE_PATH}/tests.info --output-directory ${COVERAGE_PATH}/ --branch-coverage
)

message("CMAKE_C_COMPILER " ${CMAKE_C_COMPILER})
message("CMAKE_CXX_COMPILER " ${CMAKE_CXX_COMPILER})
message("CMAKE_ASM_COMPILER " ${CMAKE_ASM_COMPILER})

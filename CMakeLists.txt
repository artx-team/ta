cmake_minimum_required(VERSION 3.21)
project(ta C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTING "Build tests" OFF)

option(ENABLE_VALGRIND "Run tests with valgrind" OFF)
option(ENABLE_ANALYZER "Enable -fanalyzer compiler flag" OFF)
option(ENABLE_COVERAGE "Enable --coverage compiler flag" OFF)

add_subdirectory(src)

find_program(ASTYLE astyle)
if(ASTYLE)
    add_custom_target(astyle
        VERBATIM
        USES_TERMINAL
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${ASTYLE} --options=${CMAKE_SOURCE_DIR}/.astylerc -Q -n -r
            "src/*.c" "src/*.h"
    )
endif()

cmake_minimum_required(VERSION 3.21)
project(ta C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                 "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTS "Build tests" OFF)
option(ENABLE_VALGRIND "Run tests with valgrind" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb3")
set(CMAKE_C_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -ggdb -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -g0 -DNDEBUG")

set(TA_C_FLAGS
    -pipe
    -funwind-tables
    -fno-common
    -Werror
    -Wfatal-errors
    -Wall
    -Wextra
    -Wundef
    -pedantic
    -pedantic-errors
    -fvisibility=hidden
    -Wmissing-prototypes
    -Wstrict-prototypes
    -Wdouble-promotion
    -Wconversion
    -Wundef
)

add_subdirectory(src)

find_program(ASTYLE astyle)
if(ASTYLE)
    add_custom_target(astyle
        VERBATIM
        USES_TERMINAL
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMAND ${ASTYLE} --options=${CMAKE_SOURCE_DIR}/.astylerc -Q -n -r
            "src/*.c" "src/*.h" "test/*.c"
    )
endif()

if(BUILD_TESTS)
    include(CTest)
    find_package(CUnit REQUIRED)

    if (ENABLE_VALGRIND)
        find_program(VALGRIND valgrind)
        if(VALGRIND)
            set(VALGRIND_OPTIONS
                --tool=memcheck
                --leak-check=full
                --show-leak-kinds=all
                --track-fds=no
                --track-origins=yes
                --trace-children=yes
                --error-exitcode=1
            )
        endif()
    endif()

    add_subdirectory(test)
endif(BUILD_TESTS)

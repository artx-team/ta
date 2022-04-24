cmake_minimum_required(VERSION 3.21)
project(ta C)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules/")

include(GNUInstallDirs)

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_TESTS "Build tests" OFF)
option(ENABLE_VALGRIND "Run tests with valgrind" OFF)
option(ENABLE_ANALYZER "Enable -fanalyzer compiler flag" OFF)
option(ENABLE_COVERAGE "Enable --coverage compiler flag" OFF)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_FLAGS_DEBUG "-O0 -ggdb3")
set(CMAKE_C_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -ggdb -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -g0 -DNDEBUG")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                 "FullDebug" "Debug" "Release" "MinSizeRel" "RelWithDebInfo" "ASAN" "MSAN")
endif()

set(CMAKE_C_FLAGS_FULLDEBUG "${CMAKE_C_FLAGS_DEBUG}")
foreach(FLAG
    -fno-common
    -fno-omit-frame-pointer
    -fno-optimize-sibling-calls
    -fstack-protector-all
)
    set(CMAKE_C_FLAGS_FULLDEBUG "${CMAKE_C_FLAGS_DEBUG} ${FLAG}")
endforeach()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS_FULLDEBUG "${CMAKE_C_FLAGS_FULLDEBUG} -fvar-tracking -fvar-tracking-assignments")
endif()

set(CMAKE_C_FLAGS_ASAN "${CMAKE_C_FLAGS_FULLDEBUG} -O1")
set(CMAKE_C_FLAGS_MSAN "${CMAKE_C_FLAGS_FULLDEBUG} -O1")

foreach(FLAG
    -fsanitize=address
    -fsanitize-address-use-after-scope
    -fsanitize=undefined
    -fsanitize-undefined-trap-on-error
    -fno-sanitize-recover=all
)
    set(CMAKE_C_FLAGS_ASAN "${CMAKE_C_FLAGS_ASAN} ${FLAG}")
endforeach()

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_C_FLAGS_ASAN "${CMAKE_C_FLAGS_ASAN} -fsanitize=pointer-compare -fsanitize=pointer-subtract")
    set(CMAKE_C_FLAGS_MSAN "${CMAKE_C_FLAGS_MSAN} -fsanitize=leak")
endif()

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS_MSAN "${CMAKE_C_FLAGS_MSAN} -fsanitize=memory -fsanitize-memory-track-origins")
endif()

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

if(ENABLE_ANALYZER AND CMAKE_C_COMPILER_ID STREQUAL "GNU")
    list(APPEND TA_C_FLAGS -fanalyzer)
endif()

if(ENABLE_COVERAGE)
    list(APPEND TA_C_FLAGS --coverage)
endif()

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

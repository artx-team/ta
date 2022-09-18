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

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
                 "FullDebug" "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
                 "ASAN" "MSAN")
endif()

if(NOT DEFINED CMAKE_POSITION_INDEPENDENT_CODE)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()

if(NOT DEFINED CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
elseif(CMAKE_INTERPROCEDURAL_OPTIMIZATION)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT IPO_SUPPORTED LANGUAGES C)
    if(NOT IPO_SUPPORTED)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION OFF)
    endif()
endif()

set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-O3 -g0 -DNDEBUG")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "-O3 -g -DNDEBUG")
set(CMAKE_C_FLAGS_MINSIZEREL "-Os -g0 -DNDEBUG")

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
    set(CMAKE_C_FLAGS_FULLDEBUG "${CMAKE_C_FLAGS_FULLDEBUG} -fvar-tracking")
    set(CMAKE_C_FLAGS_FULLDEBUG "${CMAKE_C_FLAGS_FULLDEBUG} -fvar-tracking-assignments")
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
    set(CMAKE_C_FLAGS_ASAN "${CMAKE_C_FLAGS_ASAN} -fsanitize=pointer-compare")
    set(CMAKE_C_FLAGS_ASAN "${CMAKE_C_FLAGS_ASAN} -fsanitize=pointer-subtract")
    set(CMAKE_C_FLAGS_MSAN "${CMAKE_C_FLAGS_MSAN} -fsanitize=leak")
elseif(CMAKE_C_COMPILER_ID MATCHES "Clang")
    set(CMAKE_C_FLAGS_MSAN "${CMAKE_C_FLAGS_MSAN} -fsanitize=memory")
    set(CMAKE_C_FLAGS_MSAN "${CMAKE_C_FLAGS_MSAN} -fsanitize-memory-track-origins")
endif()

set(GLOBAL_C_FLAGS
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
    -D_GNU_SOURCE
    -D_LARGE_FILES
    -D_LARGEFILE_SOURCE
    -D_FILE_OFFSET_BITS=64
    -D_TIME_BITS=64
)

set(CHECK_C_FLAGS
    -Wstrict-prototypes
    -Wmissing-prototypes
    -Wmissing-declarations
    -Wno-missing-field-initializers
    -Wno-unused-parameter
    -Wduplicated-branches
    -Wduplicated-cond
    -Wimplicit-fallthrough
    -Wdouble-promotion
    -Wpointer-arith
    -Wfloat-equal
    -Wlogical-op
    -Wshadow
    -Wvla
    -Walloca
    -Wformat=2
    -Wformat-overflow=2
    -Wformat-truncation=2
    -Wformat-signedness
    -Wno-format-pedantic
    -Wno-format-nonliteral
    -Wno-clobbered
)

if(CMAKE_C_COMPILER_ID MATCHES "Clang")
    list(APPEND CHECK_C_FLAGS
        -Wno-gnu
        -Wno-builtin-requires-header
        -Wno-missing-braces
    )
endif()

if(ENABLE_ANALYZER AND CMAKE_C_COMPILER_ID STREQUAL "GNU")
    list(APPEND CHECK_C_FLAGS -fanalyzer)
    if(CMAKE_C_COMPILER_VERSION VERSION_LESS "12.0.0")
        list(APPEND CHECK_C_FLAGS -Wno-analyzer-mismatching-deallocation)
    endif()
endif()

if(ENABLE_COVERAGE)
    list(APPEND CHECK_C_FLAGS --coverage)
endif()

include(CheckCCompilerFlag)
set(CMAKE_REQUIRED_QUIET_BAK ${CMAKE_REQUIRED_QUIET})
set(CMAKE_REQUIRED_QUIET ON)

foreach(FLAG ${CHECK_C_FLAGS})
    message(STATUS "Performing Test ${FLAG}")
    string(REGEX REPLACE "[^A-Za-z0-9_]" "_" C_FLAG ${FLAG})
    check_c_compiler_flag(${FLAG} C_FLAG_${C_FLAG})
    if(C_FLAG_${C_FLAG})
        message(STATUS "Performing Test ${FLAG} - Success")
        list(APPEND GLOBAL_C_FLAGS ${FLAG})
    else()
        message(STATUS "Performing Test ${FLAG} - Failed")
    endif()
    unset(C_FLAG_${C_FLAG})
endforeach()

set(CMAKE_REQUIRED_QUIET ${CMAKE_REQUIRED_QUIET_BAK})
unset(CMAKE_REQUIRED_QUIET_BAK)

foreach(FLAG ${GLOBAL_C_FLAGS})
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${FLAG}")
endforeach()

string(STRIP "${CMAKE_C_FLAGS}" CMAKE_C_FLAGS)

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

    if(ENABLE_VALGRIND)
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

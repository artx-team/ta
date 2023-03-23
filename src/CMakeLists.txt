include(GNUInstallDirs)
include(ProcessorCount)
ProcessorCount(MAKEJOBS)
if(MAKEJOBS LESS_EQUAL 0)
    set(MAKEJOBS 1)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_C_VISIBILITY_PRESET "hidden")
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "FullDebug" "Debug" "Release" "MinSizeRel" "RelWithDebInfo" "ASAN" "MSAN")
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
    -Wc++-compat
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
    list(PREPEND GLOBAL_C_FLAGS --coverage)
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

add_library(ta ta.c)
target_include_directories(ta BEFORE PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
install(TARGETS ta DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ta.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

if(BUILD_TESTING)
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

    enable_testing()

    add_executable(ta_test EXCLUDE_FROM_ALL ta_test.c)
    target_link_libraries(ta_test PRIVATE ta ${CUNIT_LIBRARIES})
    target_include_directories(ta_test BEFORE PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CUNIT_INCLUDE_DIRS}
    )

    if(VALGRIND)
        add_test(NAME ta_test COMMAND ${VALGRIND} ${VALGRIND_OPTIONS}
                 ${CMAKE_CURRENT_BINARY_DIR}/ta_test)
    else()
        add_test(NAME ta_test COMMAND
                 ${CMAKE_CURRENT_BINARY_DIR}/ta_test)
    endif()

    add_custom_target(check
        VERBATIM
        USES_TERMINAL
        DEPENDS ta_test
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMAND ${CMAKE_CTEST_COMMAND} -C ${CMAKE_BUILD_TYPE} -j${MAKEJOBS}
            --progress --verbose --output-on-failure
    )
endif(BUILD_TESTING)

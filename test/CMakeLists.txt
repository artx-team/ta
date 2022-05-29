add_executable(test_ta test_ta.c)
target_link_libraries(test_ta PRIVATE ta ${CUNIT_LIBRARIES})
target_compile_options(test_ta PRIVATE ${TA_C_FLAGS})
target_include_directories(test_ta BEFORE PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CUNIT_INCLUDE_DIRS}
)

if(VALGRIND)
    add_test(NAME test_ta
             COMMAND ${VALGRIND} ${VALGRIND_OPTIONS} ${CMAKE_CURRENT_BINARY_DIR}/test_ta)
else()
    add_test(NAME test_ta
             COMMAND test_ta)
endif()

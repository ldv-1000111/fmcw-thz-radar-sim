add_executable(radar_tests
    test_if_signal.cpp
    test_micro_doppler.cpp
)

target_link_libraries(radar_tests
    PRIVATE fmcw_core
            Catch2::Catch2WithMain
)

target_compile_options(radar_tests PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Run the entire test binary as a single ctest entry.
# Catch2 reports each TEST_CASE internally with full pass/fail detail.
#
# DO NOT use individual add_test() calls with test names here.
# When ctest passes a test name as a command argument it wraps it in
# double-quote characters. Catch2 receives the literal string
# '"IF signal ..."' (with quotes) and finds no matching test case,
# printing "No test cases matched" for every test.
#
# This single-entry approach works identically with Unix Makefiles and Ninja.
add_test(
    NAME radar_unit_tests
    COMMAND radar_tests
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

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

include(CTest)
include(Catch)

# DISCOVERY_MODE PRE_TEST: discover tests at ctest run time, not at cmake
# configure time. This prevents the "No tests were found" warning that appears
# when cmake runs before the binary has been built.
catch_discover_tests(radar_tests
    DISCOVERY_MODE PRE_TEST
)

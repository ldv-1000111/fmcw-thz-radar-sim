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

# Register each test with add_test using the exact name Catch2 uses.
# Names use em-dash (—) matching the TEST_CASE strings in the source.
# add_test works with both Ninja and Unix Makefiles generators.
add_test(NAME "IF signal — range bin accuracy within 1 bin"
    COMMAND radar_tests "IF signal — range bin accuracy within 1 bin"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME "IF signal — output buffer size equals num_samples"
    COMMAND radar_tests "IF signal — output buffer size equals num_samples"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME "IF signal — all samples have unit magnitude"
    COMMAND radar_tests "IF signal — all samples have unit magnitude"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME "IF signal — chirp index shifts slow-time phase"
    COMMAND radar_tests "IF signal — chirp index shifts slow-time phase"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME "Micro-Doppler — vibration increases slow-time phase variance"
    COMMAND radar_tests "Micro-Doppler — vibration increases slow-time phase variance"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME "Micro-Doppler — zero amplitude equals static model"
    COMMAND radar_tests "Micro-Doppler — zero amplitude equals static model"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_test(NAME "Micro-Doppler — larger amplitude increases phase spread"
    COMMAND radar_tests "Micro-Doppler — larger amplitude increases phase spread"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

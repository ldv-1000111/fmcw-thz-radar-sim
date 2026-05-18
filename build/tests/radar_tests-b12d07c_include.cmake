if(EXISTS "/home/lvs/data/1_devel/10_claudeCode/05_fmcw/fmcw-thz-radar-sim/build/tests/radar_tests")
  if(NOT EXISTS "/home/lvs/data/1_devel/10_claudeCode/05_fmcw/fmcw-thz-radar-sim/build/tests/radar_tests-b12d07c_tests.cmake" OR
     NOT "/home/lvs/data/1_devel/10_claudeCode/05_fmcw/fmcw-thz-radar-sim/build/tests/radar_tests-b12d07c_tests.cmake" IS_NEWER_THAN "/home/lvs/data/1_devel/10_claudeCode/05_fmcw/fmcw-thz-radar-sim/build/tests/radar_tests" OR
     NOT "/home/lvs/data/1_devel/10_claudeCode/05_fmcw/fmcw-thz-radar-sim/build/tests/radar_tests-b12d07c_tests.cmake" IS_NEWER_THAN "${CMAKE_CURRENT_LIST_FILE}")
    include("/home/lvs/data/1_devel/10_claudeCode/05_fmcw/fmcw-thz-radar-sim/build/_deps/catch2-src/extras/CatchAddTests.cmake")
    catch_discover_tests_impl(
      TEST_EXECUTABLE [==[/home/lvs/data/1_devel/10_claudeCode/05_fmcw/fmcw-thz-radar-sim/build/tests/radar_tests]==]
      TEST_EXECUTOR [==[]==]
      TEST_WORKING_DIR [==[/home/lvs/data/1_devel/10_claudeCode/05_fmcw/fmcw-thz-radar-sim/build/tests]==]
      TEST_SPEC [==[]==]
      TEST_EXTRA_ARGS [==[]==]
      TEST_PROPERTIES [==[]==]
      TEST_PREFIX [==[]==]
      TEST_SUFFIX [==[]==]
      TEST_LIST [==[radar_tests_TESTS]==]
      TEST_REPORTER [==[]==]
      TEST_OUTPUT_DIR [==[]==]
      TEST_OUTPUT_PREFIX [==[]==]
      TEST_OUTPUT_SUFFIX [==[]==]
      CTEST_FILE [==[/home/lvs/data/1_devel/10_claudeCode/05_fmcw/fmcw-thz-radar-sim/build/tests/radar_tests-b12d07c_tests.cmake]==]
      TEST_DL_PATHS [==[]==]
      CTEST_FILE [==[]==]
    )
  endif()
  include("/home/lvs/data/1_devel/10_claudeCode/05_fmcw/fmcw-thz-radar-sim/build/tests/radar_tests-b12d07c_tests.cmake")
else()
  add_test(radar_tests_NOT_BUILT radar_tests_NOT_BUILT)
endif()

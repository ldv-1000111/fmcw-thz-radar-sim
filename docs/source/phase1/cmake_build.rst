.. _phase1_cmake_build:

CMake Build System
===================

The root ``CMakeLists.txt`` builds the ``fmcw_core`` static library, the
``radar_sim`` executable, and the Catch2 test suite.
Catch2 is fetched automatically via ``FetchContent`` — no manual dependency
installation is required on the developer machine or in CI.

Root ``CMakeLists.txt``
------------------------

.. code-block:: cmake
   :caption: CMakeLists.txt
   :linenos:

   cmake_minimum_required(VERSION 3.24)
   project(fmcw_thz_radar_sim VERSION 0.1.0 LANGUAGES CXX)

   set(CMAKE_CXX_STANDARD 17)
   set(CMAKE_CXX_STANDARD_REQUIRED ON)
   set(CMAKE_CXX_EXTENSIONS OFF)

   # ── Compiler warnings (treated as errors) ────────────────────
   if(MSVC)
     add_compile_options(/W4 /WX)
   else()
     add_compile_options(-Wall -Wextra -Wpedantic -Werror)
   endif()

   # ── fmcw_core static library ─────────────────────────────────
   add_library(fmcw_core STATIC
       src/fmcw_generator.cpp
   )
   target_include_directories(fmcw_core PUBLIC include)

   # ── Main executable ──────────────────────────────────────────
   add_executable(radar_sim src/main.cpp)
   target_link_libraries(radar_sim PRIVATE fmcw_core)

   # ── Catch2 test suite (FetchContent — no manual install) ─────
   include(FetchContent)
   FetchContent_Declare(
       Catch2
       GIT_REPOSITORY https://github.com/catchorg/Catch2.git
       GIT_TAG        v3.6.0
   )
   FetchContent_MakeAvailable(Catch2)

   enable_testing()
   add_subdirectory(tests)

   # ── Install rules ────────────────────────────────────────────
   install(TARGETS radar_sim DESTINATION bin)
   install(DIRECTORY include/ DESTINATION include)

``tests/CMakeLists.txt``
-------------------------

.. code-block:: cmake
   :caption: tests/CMakeLists.txt
   :linenos:

   add_executable(radar_tests
       test_if_signal.cpp
       test_micro_doppler.cpp
   )

   target_link_libraries(radar_tests
       PRIVATE fmcw_core
               Catch2::Catch2WithMain
   )

   include(CTest)
   include(Catch)
   catch_discover_tests(radar_tests)

Build Commands
---------------

.. code-block:: bash

   # Configure (Release, Ninja generator)
   cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

   # Build everything
   cmake --build build --parallel

   # Run all tests
   ctest --test-dir build --output-on-failure -V

   # Run the binary
   ./build/radar_sim
   # → Phase 1: wrote 5000 samples to if_signal.csv

   # Install to /usr/local (optional)
   cmake --install build --prefix /usr/local

.. tip::

   For faster iteration during development use ``Debug`` mode and
   ``--parallel`` to exploit all CPU cores:

   .. code-block:: bash

      cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
      cmake --build build --parallel

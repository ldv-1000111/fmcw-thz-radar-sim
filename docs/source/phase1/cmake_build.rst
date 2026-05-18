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

   # CMP0135: use archive extraction timestamp for FetchContent, not the
   # timestamp of each extracted file. Prevents make/ninja from treating
   # freshly extracted Catch2 sources as newer than the build system manifest.
   if(POLICY CMP0135)
       cmake_policy(SET CMP0135 NEW)
   endif()

   # -- Compiler warnings
   if(MSVC)
     add_compile_options(/W4 /WX)
   else()
     add_compile_options(-Wall -Wextra -Wpedantic -Werror)
   endif()

   # -- fmcw_core static library
   add_library(fmcw_core STATIC
       src/fmcw_generator.cpp
   )
   target_include_directories(fmcw_core PUBLIC include)

   # -- Main executable
   add_executable(radar_sim src/main.cpp)
   target_link_libraries(radar_sim PRIVATE fmcw_core)

   # -- Catch2 via FetchContent
   include(FetchContent)
   FetchContent_Declare(
       Catch2
       GIT_REPOSITORY https://github.com/catchorg/Catch2.git
       GIT_TAG        v3.6.0
       GIT_SHALLOW    TRUE
   )
   FetchContent_MakeAvailable(Catch2)

   enable_testing()

   # Unique binary dir name avoids conflict with Catch2's own tests/ subdirectory
   add_subdirectory(tests radar_tests_build)

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

   target_compile_options(radar_tests PRIVATE
       $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
       $<$<CXX_COMPILER_ID:MSVC>:/W4>
   )

   # Run the entire test binary as a single ctest entry.
   # Catch2 reports each TEST_CASE internally with full pass/fail detail.
   # Running individual tests via add_test causes ctest to quote the test
   # name, which Catch2 misinterprets as a literal filter pattern
   # (e.g. searching for '"IF signal..."' including the quote characters).
   add_test(
       NAME radar_unit_tests
       COMMAND radar_tests
       WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
   )
Build Commands
---------------

.. code-block:: bash

   # Configure (Release, Unix Makefiles generator)
   cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release

   # Build everything
   cmake --build build --parallel

   # Run all tests
   ctest --test-dir build --output-on-failure -V

   # Run the binary
   ./build/radar_sim

   # Install to /usr/local (optional)
   cmake --install build --prefix /usr/local

Troubleshooting — Ninja Manifest Dirty Error
---------------------------------------------

If you see:

.. code-block:: text

   ninja: error: manifest 'build.ninja' still dirty after 100 tries,
   perhaps system time is not set

This is a **Ninja timestamp resolution** issue, not a code problem. Three
known causes on native Linux:

**Cause 1 — Build directory is on a low-resolution filesystem**

``tmpfs``, NFS, CIFS, and some FUSE mounts use 1-second timestamp
resolution. Ninja writes ``build.ninja`` and immediately reads back a
timestamp that looks equal to or older than the source files it just
processed.

.. code-block:: bash

   # Check the filesystem under your build directory
   df -Th /path/to/fmcw-thz-radar-sim

   # If it shows tmpfs / nfs / cifs / fuse, move the build elsewhere:
   cmake -B /tmp/fmcw-build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release \
         -S /path/to/fmcw-thz-radar-sim
   cmake --build /tmp/fmcw-build --parallel

**Cause 2 — Ninja version older than 1.11**

Ninja < 1.11 has a known bug where the ``re-running CMake`` step after
``FetchContent`` populates new files triggers this loop.

.. code-block:: bash

   ninja --version   # must be >= 1.11
   sudo apt install ninja-build   # Ubuntu 22.04+ provides 1.11

**Cause 3 — Docker bind-mount with sub-second clock skew**

Host and container clocks can differ by sub-second amounts that
confuse Ninja.

.. code-block:: bash

   systemd-detect-virt   # confirms if running in a container

**Universal fallback — switch to Unix Makefiles**

``Unix Makefiles`` does not use ``build.ninja`` and is immune to all
three causes above:

.. code-block:: bash

   rm -rf build
   cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release
   cmake --build build --parallel $(nproc)
   ctest --test-dir build --output-on-failure -V

.. tip::

   For faster iteration during development use ``Debug`` mode:

   .. code-block:: bash

      cmake -B build -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Debug
      cmake --build build --parallel

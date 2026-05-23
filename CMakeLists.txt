cmake_minimum_required(VERSION 3.24)
project(fmcw_thz_radar_sim VERSION 0.2.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# CMP0135: use archive extraction timestamp for FetchContent
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

# -- Compiler warnings
if(MSVC)
    add_compile_options(/W4 /WX)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# -- FFTW3 single-precision (Phase 2)
# Install: sudo apt install libfftw3-dev
find_package(PkgConfig REQUIRED)
pkg_check_modules(FFTW3F REQUIRED fftw3f)

# -- fmcw_core static library (Phase 1 + Phase 2)
add_library(fmcw_core STATIC
    src/fmcw_generator.cpp
    src/signal_processing.cpp
    src/cfar.cpp
)
target_include_directories(fmcw_core PUBLIC include)
target_link_libraries(fmcw_core PUBLIC ${FFTW3F_LIBRARIES})
target_compile_options(fmcw_core PRIVATE ${FFTW3F_CFLAGS_OTHER})

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
add_subdirectory(tests radar_tests_build)

# -- Install rules
install(TARGETS radar_sim DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)

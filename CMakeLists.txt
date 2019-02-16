cmake_minimum_required(VERSION 3.8 FATAL_ERROR) # note: cmake 3.8 needed for clang-tidy
cmake_policy(SET CMP0054 NEW)

project(parsers)

include(cmake-utils/utils.cmake)

require_gcc_version(5.0)
require_clang_version(4.0)

# Default value is not to build test cases
option(BUILD_TESTING "build test cases" ON)

# The following options are helpful for development and/or CI
option(USE_WERROR "build with -Werror flag")
option(USE_CLANG_TIDY "build with clang-tidy checks enabled" OFF)
option(USE_IWYU "build with iwyu checks enabled" OFF)
option(CLANG_TIDY_WARNINGS_AS_ERRORS "treat clang-tidy warnings as errors" OFF)

# Default value is to build in release mode but with debug symbols
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE INTERNAL "CMAKE_BUILD_TYPE")
endif(NOT CMAKE_BUILD_TYPE)

# The MSVC version on AppVeyor CI needs this
if(MSVC)
  add_definitions(/bigobj)
endif()

add_subdirectory(vendor)
add_subdirectory(src)
add_subdirectory(test)

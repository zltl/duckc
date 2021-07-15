cmake_minimum_required(VERSION 3.9)

project(
  duckc
  VERSION 0.1
  LANGUAGES C CXX)

option(DUCKC_BUILD_TEST "Build unit test" ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

# C standard can be overridden when this is used as a sub-project.
if(NOT CMAKE_C_STANDARD)
  # This project can use C11, but will gracefully decay down to C89.
  set(CMAKE_C_STANDARD 11)
  set(CMAKE_C_STANDARD_REQUIRED OFF)
  set(CMAKE_C_EXTENSIONS OFF)
endif(NOT CMAKE_C_STANDARD)

# C++ standard can be overridden when this is used as a sub-project.
if(NOT CMAKE_CXX_STANDARD)
  # This project requires C++17.
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif(NOT CMAKE_CXX_STANDARD)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(ImportLibrary)
find_package(Threads)

# build library
set(mylib_name duckc)
file(GLOB lib_SOURCE_FILES "*.c" "*.h" "include/*.h")
include_directories(${CMAKE_CURRENT_SOURCE_DIR}
                    ${CMAKE_CURRENT_SOURCE_DIR}/include)
add_library(${mylib_name} ${lib_SOURCE_FILES})
set_target_properties(
  ${mylib_name} PROPERTIES VERSION ${PROJECT_VERSION} SOVERSION
                                                      ${PROJECT_VERSION_MAJOR})
set_target_properties(
  ${mylib_name} PROPERTIES INTERFACE_INCLUDE_DIRECTORIES
  ${CMAKE_CURRENT_SOURCE_DIR}/include/)
target_compile_options(
  ${mylib_name} PUBLIC -Werror -Wall -Wextra -pedantic
  -Wl,-export-dynamic -Wundef -Wvla)

# test
if(DUCKC_BUILD_TEST)
  # download google test
  import_library(
    googletest
    https://github.com/google/googletest/archive/refs/tags/release-1.11.0.zip
    52943a59cefce0ae0491d4d2412c120b)
  add_subdirectory(${IMPORT_SRC} ${IMPORT_BUILD} EXCLUDE_FROM_ALL)
  add_subdirectory(tests)
endif(DUCKC_BUILD_TEST)

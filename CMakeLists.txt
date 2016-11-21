set(CMAKE_MACOSX_RPATH 1)
cmake_minimum_required(VERSION 2.8)
project(libnet)

enable_testing()


set(CMAKE_CXX_COMPILER "g++")
set(CMAKE_CXX_FLAGS "-std=c++11 -pthread")

SET(CMAKE_CXX_FLAGS_DEBUG "-O0 -Wall -g")
SET(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall -DNDEBUG")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

#cmake -DCMAKE_BUILD_TYPE=Debug/Release path
 
message(STATUS " project source directory = " ${PROJECT_SOURCE_DIR})
message(STATUS " CMAKE_BUILD_TYPE = " ${CMAKE_BUILD_TYPE})
string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS " CMAKE_CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})


find_path(GTEST_INCLUDE_DIR gtest/gtest.h)
message(STATUS "found GTEST_INCLUDE_DIR " ${GTEST_INCLUDE_DIR})
include_directories(${GTEST_INCLUDE_DIR})
find_library(GTEST_LIBRARY NAMES gtest)
message(STATUS "found GTEST_LIBRARY " ${GTEST_LIBRARY})

include_directories(${GTEST_INCLUDE_DIR})

find_path(PROFILER_INCLUDE_DIR gperftools/profiler.h)
find_library(PROFILER_LIBRARY NAMES profiler)
find_path(TCMALLOC_INCLUDE_DIR gperftools/heap-profiler.h)
find_library(TCMALLOC_LIBRARY NAMES tcmalloc)


if(PROFILER_INCLUDE_DIR AND PROFILER_LIBRARY)
message(STATUS "found profiler " ${PROFILER_INCLUDE_DIR} ${PROFILER_LIBRARY})
endif()


if(TCMALLOC_INCLUDE_DIR AND TCMALLOC_LIBRARY)
message(STATUS "found tcmalloc" ${TCMALLOC_INCLUDE_DIR} ${TCMALLOC_LIBRARY})
endif()

add_subdirectory(libnet)
add_subdirectory(example)

# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.20)

include(cmake/platform.cmake)
include(cmake/coverage.cmake)
include(cmake/checkGit.cmake)

get_git_commit()

project(uniLibRSU
        VERSION 0.9
        LANGUAGES ASM C CXX
        DESCRIPTION "unified Remote System Update for Agilex platform")

message("-- PROJECT NAME      : ${PROJECT_NAME}")
message("-- PROJECT VERSION   : ${PROJECT_VERSION}")
message("-- PROJECT GIT SHA1  : ${GIT_HASH}")

include(cmake/doxygen.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(UNIT_TEST "Enable Unit testing of libRSU" OFF)
option(SHARED_LIB "Enable building of shared library" OFF)

if(${PLATFORM} STREQUAL host AND UNIT_TEST EQUAL ON)
    set(BUILD_DOC OFF)
endif()

if(COVERAGE)
  add_lcov_coverage(IGNORE *unit-test*)
endif()

if(SHARED_LIB)
  set(LIBRARY_TYPE SHARED)
else()
  set(LIBRARY_TYPE STATIC)
endif()

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

add_library(uniLibRSU ${LIBRARY_TYPE} "")

add_subdirectory(platform)

#add library source and include
add_subdirectory(include)
add_subdirectory(src)

add_subdirectory(version)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

if(UNIT_TEST)
    enable_testing()
    add_subdirectory(unit-test)
endif()

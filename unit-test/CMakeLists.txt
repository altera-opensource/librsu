# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.24)

if(NOT "${PLATFORM}" STREQUAL "host")
  message(FATAL_ERROR "PLATFORM is not host")
endif()

include(FetchContent)
include(FindZLIB)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

if(NOT ZLIB_FOUND)
  FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG v1.3.1
    FIND_PACKAGE_ARGS
  )
  FetchContent_MakeAvailable(zlib)
else()
  message("-- zlib found at: " ${ZLIB_LIBRARIES} " and include at:" ${ZLIB_INCLUDE_DIRS})
endif()


add_subdirectory(collaterals)
add_subdirectory(test1)
add_subdirectory(test2)
add_subdirectory(test3)
add_subdirectory(test4)
add_subdirectory(test5)
add_subdirectory(test6)

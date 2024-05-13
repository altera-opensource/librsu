# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.20)

if(NOT "${PLATFORM}" STREQUAL "host")
  message(FATAL_ERROR "PLATFORM is not host")
endif()

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG v1.3.1
)

FetchContent_MakeAvailable(zlib)

target_include_directories(zlib PUBLIC
    $<BUILD_INTERFACE:${zlib_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(zlibstatic PUBLIC
    $<BUILD_INTERFACE:${zlib_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(zlib PUBLIC
    $<BUILD_INTERFACE:${zlib_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)
target_include_directories(zlibstatic PUBLIC
    $<BUILD_INTERFACE:${zlib_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

add_subdirectory(collaterals)
add_subdirectory(test1)
add_subdirectory(test2)
add_subdirectory(test3)
add_subdirectory(test4)
add_subdirectory(test5)
add_subdirectory(test6)

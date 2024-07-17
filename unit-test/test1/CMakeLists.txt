# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.24)
add_executable(librsu_test librsu_test.cpp)
target_include_directories(librsu_test
PRIVATE "${PROJECT_BINARY_DIR}/version"
)
add_subdirectory(dependency)

target_link_libraries(
librsu_test
PRIVATE
GTest::gtest_main
uniLibRSU
)

include(GoogleTest)
gtest_discover_tests(librsu_test)

if(COVERAGE)
    add_dependencies(coverage librsu_test)
endif()

configure_file("librsu_config.rc" "librsu_config.rc" COPYONLY)

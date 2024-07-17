# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.24)
add_executable(librsu_test4 librsu_test.cpp)

add_subdirectory(dependency)

target_link_libraries(
librsu_test4
PRIVATE
GTest::gtest_main
uniLibRSU
)

include(GoogleTest)
gtest_discover_tests(librsu_test4)

if(COVERAGE)
    add_dependencies(coverage librsu_test4)
endif()

configure_file("librsu_config.rc" "librsu_config.rc" COPYONLY)

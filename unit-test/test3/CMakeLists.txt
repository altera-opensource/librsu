# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.24)
add_executable(librsu_test3 librsu_test3.cpp)

add_subdirectory(dependency3)

target_link_libraries(
librsu_test3
PRIVATE
GTest::gtest_main
uniLibRSU
)

include(GoogleTest)
gtest_discover_tests(librsu_test3)

if(COVERAGE)
    add_dependencies(coverage librsu_test3)
endif()

configure_file("librsu_config.rc" "librsu_config.rc" COPYONLY)

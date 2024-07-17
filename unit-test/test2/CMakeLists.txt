# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.24)
add_executable(librsu_test2 librsu_test2.cpp)

add_subdirectory(dependency2)

target_link_libraries(
librsu_test2
PRIVATE
GTest::gtest_main
uniLibRSU
)

include(GoogleTest)
gtest_discover_tests(librsu_test2)

if(COVERAGE)
    add_dependencies(coverage librsu_test2)
endif()

configure_file("librsu_config.rc" "librsu_config.rc" COPYONLY)

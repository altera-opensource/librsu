# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.24)

target_include_directories(uniLibRSU PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/RSU_osal_linux.c")
target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/RSU_logging_linux.c")

set_target_properties(uniLibRSU PROPERTIES PUBLIC_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/RSU_OSAL_types.h")

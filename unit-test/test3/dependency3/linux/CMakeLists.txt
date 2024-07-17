# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.24)

target_sources(librsu_test3 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/RSU_osal_linux.c")
target_sources(librsu_test3 PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/RSU_logging_linux.c")


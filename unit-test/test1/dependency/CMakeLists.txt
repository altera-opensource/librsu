# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.20)
add_subdirectory(linux)
target_sources(librsu_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_crc32.c")
target_sources(librsu_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_mock_mailbox.c")
target_sources(librsu_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_mock_qspi.c")
target_sources(librsu_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_mock_file.c")
target_sources(librsu_test PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_mock_misc.c")
target_link_libraries(librsu_test PRIVATE zlibstatic)

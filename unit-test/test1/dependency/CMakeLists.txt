# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.24)
add_subdirectory(linux)
target_sources(librsu_test PRIVATE "rsu_crc32.c")
target_sources(librsu_test PRIVATE "rsu_mock_mailbox.c")
target_sources(librsu_test PRIVATE "rsu_mock_qspi.c")
target_sources(librsu_test PRIVATE "rsu_mock_file.c")
target_sources(librsu_test PRIVATE "rsu_mock_misc.c")
target_link_libraries(librsu_test LINK_PRIVATE ${ZLIB_LIBRARIES})

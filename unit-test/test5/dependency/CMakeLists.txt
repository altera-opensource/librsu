# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.24)
add_subdirectory(linux)
target_sources(librsu_test5 PRIVATE "rsu_crc32.c")
target_sources(librsu_test5 PRIVATE "rsu_mock_mailbox.c")
target_sources(librsu_test5 PRIVATE "rsu_mock_qspi.c")
target_sources(librsu_test5 PRIVATE "rsu_mock_file.c")
target_sources(librsu_test5 PRIVATE "rsu_mock_misc.c")
target_include_directories(librsu_test5 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(librsu_test5 LINK_PRIVATE ${ZLIB_LIBRARIES})

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/output_file_jic.rpd ${CMAKE_CURRENT_BINARY_DIR}/fip.bin
                  COMMAND cp -v ${COLLATERALS}/output_file_jic.rpd ${CMAKE_CURRENT_BINARY_DIR}/output_file_jic.rpd
                  COMMAND cp -v ${COLLATERALS}/fip.bin ${CMAKE_CURRENT_BINARY_DIR}/fip.bin
                  DEPENDS generate_collaterals)

add_custom_target(get_collaterls_test5
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/output_file_jic.rpd ${CMAKE_CURRENT_BINARY_DIR}/fip.bin generate_collaterals)

add_dependencies(librsu_test5 get_collaterls_test5)

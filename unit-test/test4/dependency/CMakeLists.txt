# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.20)
add_subdirectory(linux)
target_sources(librsu_test4 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_crc32.c")
target_sources(librsu_test4 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_mock_mailbox.c")
target_sources(librsu_test4 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_mock_qspi.c")
target_sources(librsu_test4 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_mock_file.c")
target_sources(librsu_test4 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_mock_misc.c")
target_include_directories(librsu_test4 PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}")
target_link_libraries(librsu_test4 PRIVATE zlibstatic)

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/output_file_jic.rpd ${CMAKE_CURRENT_BINARY_DIR}/fip.bin ${CMAKE_CURRENT_BINARY_DIR}/app1.rpd ${CMAKE_CURRENT_BINARY_DIR}/app2.rpd ${CMAKE_CURRENT_BINARY_DIR}/app3.rpd ${CMAKE_CURRENT_BINARY_DIR}/factory_update.rpd
                  COMMAND cp -v ${COLLATERALS}/output_file_jic.rpd ${CMAKE_CURRENT_BINARY_DIR}/
                  COMMAND cp -v ${COLLATERALS}/fip.bin ${CMAKE_CURRENT_BINARY_DIR}/
                  COMMAND cp -v ${COLLATERALS}/app1.rpd ${CMAKE_CURRENT_BINARY_DIR}/
                  COMMAND cp -v ${COLLATERALS}/app2.rpd ${CMAKE_CURRENT_BINARY_DIR}/
                  COMMAND cp -v ${COLLATERALS}/app3.rpd ${CMAKE_CURRENT_BINARY_DIR}/
                  COMMAND cp -v ${COLLATERALS}/factory_update.rpd ${CMAKE_CURRENT_BINARY_DIR}/
                  DEPENDS generate_collaterals)

add_custom_target(get_collaterals_test4
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/output_file_jic.rpd ${CMAKE_CURRENT_BINARY_DIR}/fip.bin ${CMAKE_CURRENT_BINARY_DIR}/app1.rpd ${CMAKE_CURRENT_BINARY_DIR}/app2.rpd ${CMAKE_CURRENT_BINARY_DIR}/app3.rpd ${CMAKE_CURRENT_BINARY_DIR}/factory_update.rpd generate_collaterals)

add_dependencies(librsu_test4 get_collaterals_test4)

# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation
cmake_minimum_required(VERSION 3.20)

include(FetchContent)
FetchContent_Declare(
  zlib
  GIT_REPOSITORY https://github.com/madler/zlib.git
  GIT_TAG v1.3.1
)

FetchContent_MakeAvailable(zlib)

target_include_directories(zlib PUBLIC
    $<BUILD_INTERFACE:${zlib_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(zlibstatic PUBLIC
    $<BUILD_INTERFACE:${zlib_BINARY_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(zlib PUBLIC
    $<BUILD_INTERFACE:${zlib_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

target_include_directories(zlibstatic PUBLIC
    $<BUILD_INTERFACE:${zlib_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

add_subdirectory(linux)
target_sources(rsu_client PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_crc32.c")
target_sources(rsu_client PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_mailbox_ops.c")
target_sources(rsu_client PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_qspi_ops.c")
target_sources(rsu_client PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_file_ops.c")
target_sources(rsu_client PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_misc_ops.c")
target_sources(rsu_client PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_linux_utils.c")
target_link_libraries(rsu_client PRIVATE zlib)

set_target_properties(rsu_client
    PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

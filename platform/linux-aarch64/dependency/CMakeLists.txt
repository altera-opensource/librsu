# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation
cmake_minimum_required(VERSION 3.24)

include(FetchContent)
include(FindZLIB)

if(NOT ZLIB_FOUND)
  message("zlib not found, getting zlib from source")
  FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG v1.3.1
    FIND_PACKAGE_ARGS
  )
  FetchContent_MakeAvailable(zlib)
  target_include_directories(uniLibRSU PRIVATE zlib)
  target_link_libraries(uniLibRSU LINK_PRIVATE zlib)
else()
  message("-- zlib found at: " ${ZLIB_LIBRARIES} " and include at: " ${ZLIB_INCLUDE_DIRS} " " ${ZLIB_FOUND})
  target_include_directories(uniLibRSU PRIVATE ${ZLIB_INCLUDE_DIRS})
  target_link_libraries(uniLibRSU LINK_PRIVATE ${ZLIB_LIBRARIES})
endif()

add_subdirectory(linux)
target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_crc32.c")
target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_mailbox_ops.c")
target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_qspi_ops.c")
target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_file_ops.c")
target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_misc_ops.c")
target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/rsu_linux_utils.c")

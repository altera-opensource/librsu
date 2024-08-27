# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.24)

include(GNUInstallDirs)

add_executable(rsu_client rsu_client.c)

find_package(Threads REQUIRED)

target_link_libraries(
  rsu_client
  PRIVATE
  uniLibRSU
  ${CMAKE_THREAD_LIBS_INIT}
)

set_target_properties(rsu_client PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/lib"
  LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/lib"
  RUNTIME_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/bin"
)

install(TARGETS rsu_client
  RUNTIME
    DESTINATION ${CMAKE_INSTALL_SBINDIR}
    COMPONENT uniLibRSU_client_Runtime_GSRD
)

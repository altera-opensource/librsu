# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.20)

add_executable(rsu_client rsu_client.c)

add_subdirectory(dependency)
find_package( Threads REQUIRED )

target_link_libraries(
  rsu_client
  PRIVATE
  uniLibRSU
  ${CMAKE_THREAD_LIBS_INIT}
)

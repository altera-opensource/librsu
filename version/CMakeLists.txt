# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation


configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/version.c.in"
  "${PROJECT_BINARY_DIR}/version/version.c"
  @ONLY
)

configure_file (
  "${CMAKE_CURRENT_SOURCE_DIR}/version.h.in"
  "${PROJECT_BINARY_DIR}/version/version.h"
  @ONLY
)

target_sources(uniLibRSU PRIVATE "${PROJECT_BINARY_DIR}/version/version.c")

target_include_directories(uniLibRSU
  PRIVATE "${PROJECT_BINARY_DIR}/version/"
)

# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

set(OSAL_PLATFORM linux PARENT_SCOPE)

target_include_directories(uniLibRSU PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

set_target_properties(uniLibRSU PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY    "${CMAKE_BINARY_DIR}/bin"
)

target_compile_options(uniLibRSU PRIVATE -Wall -Wextra  -Wextra -Wno-unused-parameter
  -Wno-missing-field-initializer -Wmissing-prototypes -Wstrict-prototypes -Wold-style-definition -Wshadow
  -Wpointer-arith -Wcast-qual
)

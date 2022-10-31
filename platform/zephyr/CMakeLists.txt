# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

set(OSAL_PLATFORM zephyr PARENT_SCOPE)

target_include_directories(uniLibRSU PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")
target_link_libraries(uniLibRSU PUBLIC zephyr_interface)

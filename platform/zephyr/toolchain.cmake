# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

if(NOT ZEPHYR_TOOLCHAIN_VARIANT STREQUAL "zephyr")
    message(FATAL_ERROR "not building inside inside zephyr")
endif()
set(SHARED_LIB OFF)

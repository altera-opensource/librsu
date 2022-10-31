# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

set(CROSS_COMPILE aarch64-none-linux-gnu-)
set(SHARED_LIB ON)
if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEFAULT_TOOLCHAIN       ${CROSS_COMPILE}gcc)

    if (NOT DEFINED TOOLCHAIN)
        message(STATUS "'TOOLCHAIN' is not defined. Using '${DEFAULT_TOOLCHAIN}'")
        set(TOOLCHAIN           ${DEFAULT_TOOLCHAIN})
    endif ()

    set(CMAKE_TOOLCHAIN_FILE    ${CMAKE_CURRENT_LIST_DIR}/${TOOLCHAIN}.cmake)
endif ()

# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(DEFAULT_TOOLCHAIN       gcc)

    if (NOT DEFINED TOOLCHAIN)
        message(STATUS "'TOOLCHAIN' is not defined. Using '${DEFAULT_TOOLCHAIN}'")
        set(TOOLCHAIN           ${DEFAULT_TOOLCHAIN})
    endif ()

    set(CMAKE_TOOLCHAIN_FILE    ${CMAKE_CURRENT_LIST_DIR}/${TOOLCHAIN}.cmake)
endif ()
set(SHARED_LIB ON)
add_compile_options(-Wall -Wextra -Wpedantic)

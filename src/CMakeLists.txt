# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

target_include_directories(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/priv_include")

target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libRSU_utils.c")
target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libRSU_cfg.c")
target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libRSU.c")
target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libRSU_ops.c")
target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libRSU_misc.c")
target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libRSU_cb.c")
target_sources(uniLibRSU PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/libRSU_image.c")

target_compile_options(uniLibRSU PRIVATE -Wformat -Wformat-signedness)

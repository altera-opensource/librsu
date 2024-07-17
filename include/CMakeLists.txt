# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

target_include_directories(uniLibRSU PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
get_target_property(INTERFACE_SOURCES uniLibRSU PUBLIC_HEADER)
set_target_properties(uniLibRSU PROPERTIES PUBLIC_HEADER "${INTERFACE_SOURCES};${CMAKE_CURRENT_SOURCE_DIR}/libRSU.h;${CMAKE_CURRENT_SOURCE_DIR}/libRSU_OSAL.h")

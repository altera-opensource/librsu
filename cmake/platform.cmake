# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

if (NOT DEFINED PLATFORM)
  set(DEFAULT_PLATFORM    host)
  message(STATUS "'PLATFORM' is not defined. Using '${DEFAULT_PLATFORM}'")
  set(PLATFORM            ${DEFAULT_PLATFORM})
endif ()

# Setup platform toolchain file.
include(platform/${PLATFORM}/toolchain.cmake)

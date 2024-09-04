# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

cmake_minimum_required(VERSION 3.24)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

include(cmake/platform.cmake)
include(cmake/coverage.cmake)
include(cmake/checkGit.cmake)

get_git_commit()

project(uniLibRSU
        VERSION 1.0
        LANGUAGES ASM C CXX
        DESCRIPTION "unified Remote System Update for Agilex platform")

message("-- PROJECT NAME      : ${PROJECT_NAME}")
message("-- PROJECT VERSION   : ${PROJECT_VERSION}")
message("-- PROJECT GIT SHA1  : ${GIT_HASH}")

include(cmake/doxygen.cmake)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(UNIT_TEST "Enable Unit testing of libRSU" OFF)
option(NEED_STATIC_LIB "Override building of library as static" OFF)

if(${PLATFORM} STREQUAL host AND UNIT_TEST)
    set(BUILD_DOC OFF)
endif()

if(COVERAGE)
  add_lcov_coverage(IGNORE *unit-test*)
endif()

if(SHARED_LIB AND NOT NEED_STATIC_LIB)
  set(LIBRARY_TYPE SHARED)
else()
  set(LIBRARY_TYPE STATIC)
endif()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif ()

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_STANDARD 11)

add_library(uniLibRSU ${LIBRARY_TYPE} "")
set_target_properties(uniLibRSU PROPERTIES
                      VERSION ${PROJECT_VERSION_MAJOR}
                      SOVERSION ${PROJECT_VERSION_MAJOR})

write_basic_package_version_file(unifiedLibRSUVersion.cmake
                                  VERSION ${PROJECT_VERSION}
                                  COMPATIBILITY SameMajorVersion)

add_subdirectory(platform)

#add library source and include
add_subdirectory(include)
add_subdirectory(src)

add_subdirectory(version)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_COMPILE_WARNING_AS_ERROR ON)

set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

if(UNIT_TEST)
  enable_testing()
  add_subdirectory(unit-test)
endif()

if(PLATFORM STREQUAL linux-aarch64)

  add_subdirectory(linux-example)

  install(TARGETS uniLibRSU
  LIBRARY
    DESTINATION usr/lib
    COMPONENT uniLibRSU_lib_Development
  PUBLIC_HEADER
    DESTINATION usr/include
    COMPONENT uniLibRSU_lib_Development
  ARCHIVE
    DESTINATION usr/lib
    COMPONENT uniLibRSU_lib_Development
  )

  install(TARGETS uniLibRSU
  LIBRARY
    DESTINATION usr/lib
    COMPONENT uniLibRSU_lib_Runtime
    NAMELINK_SKIP
  ARCHIVE
    DESTINATION usr/lib
    COMPONENT uniLibRSU_lib_Runtime
  PUBLIC_HEADER
    DESTINATION usr/include
    COMPONENT uniLibRSU_lib_Runtime
  )

  install(
  FILES ${CMAKE_CURRENT_SOURCE_DIR}/etc/qspi.rc
    RENAME librsu.rc
    DESTINATION etc
    COMPONENT uniLibRSU_lib_Runtime
    PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ
  )

endif()

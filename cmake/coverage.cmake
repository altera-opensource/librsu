# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

option(COVERAGE "Generate coverage report" OFF)

function(add_lcov_coverage)
    include(CMakeParseArguments)
    include(FindPackageHandleStandardArgs)

    set(OPTIONS                 "")
    set(ONE_VALUE_ARGS          "")
    set(MULTI_VALUE_ARGS        IGNORE)
    cmake_parse_arguments(COVERAGE_OPT "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" ${ARGN})

    # Find lcov.
    find_program(LCOV_BIN lcov)
    find_package_handle_standard_args(lcov REQUIRED_VARS LCOV_BIN)
    if (NOT LCOV_FOUND)
        message(FATAL_ERROR "lcov not found")
    endif ()

    # Find genhtml.
    find_program(GENHTML_BIN genhtml)
    find_package_handle_standard_args(genhtml REQUIRED_VARS GENHTML_BIN)
    if (NOT GENHTML_FOUND)
        message(FATAL_ERROR "genhtml not found")
    endif ()

    set(COVERAGE_COMPILATION_FLAGS      --coverage -fprofile-abs-path -finstrument-functions)
    set(COVERAGE_LINKING_FLAGS          --coverage)
    set(COVERAGE_OUTPUT_PATH            ${PROJECT_BINARY_DIR}/coverage)
    set(COVERAGE_IGNORE                 '*/gcc/*' '/usr/*' '*/_deps/*')

    foreach (IGNORE_PATH IN LISTS COVERAGE_OPT_IGNORE)
        list(APPEND COVERAGE_IGNORE '${IGNORE_PATH}')
    endforeach ()

    foreach( FLAG IN LISTS COVERAGE_COMPILATION_FLAGS)
        add_compile_options(${FLAG})
    endforeach()

    foreach (FLAG IN LISTS COVERAGE_LINKING_FLAGS)
        add_link_options(${FLAG})
    endforeach()


    add_custom_target(coverage
        COMMAND ${CMAKE_COMMAND} -E make_directory ${COVERAGE_OUTPUT_PATH}
        COMMAND ${LCOV_BIN} --gcov-tool ${CMAKE_GCOV} -c -i -d . -o ${COVERAGE_OUTPUT_PATH}/base.info
        COMMAND ${LCOV_BIN} --gcov-tool ${CMAKE_GCOV} -c -d . -o ${COVERAGE_OUTPUT_PATH}/lcov.info
        COMMAND ${LCOV_BIN} --gcov-tool ${CMAKE_GCOV} -a ${COVERAGE_OUTPUT_PATH}/base.info -a ${COVERAGE_OUTPUT_PATH}/lcov.info -o ${COVERAGE_OUTPUT_PATH}/lcov.info
        COMMAND ${LCOV_BIN} --gcov-tool ${CMAKE_GCOV} -r ${COVERAGE_OUTPUT_PATH}/lcov.info ${COVERAGE_IGNORE} -o ${COVERAGE_OUTPUT_PATH}/lcov.info
    )

    add_custom_target(coverage-report
        COMMAND genhtml ${COVERAGE_OUTPUT_PATH}/lcov.info --output-directory ${COVERAGE_OUTPUT_PATH}/html
        DEPENDS coverage
    )
endfunction()


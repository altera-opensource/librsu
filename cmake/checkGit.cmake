# SPDX-License-Identifier: MIT-0
# Copyright (C) 2023-2024 Intel Corporation

function(get_git_commit)

    find_package(Git)
    if(NOT Git_FOUND)
        message("Git not found")
    endif()

    execute_process(
        COMMAND git log -1 --format=%h
        WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
        OUTPUT_VARIABLE GIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    set(GIT_HASH ${GIT_HASH} PARENT_SCOPE)

endfunction()

/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

/**
 *
 * @file RSU_utils.h
 * @brief utility functions used inside LibRSU
 */

#ifndef RSU_UTILS_H
#define RSU_UTILS_H

#include <libRSU_OSAL.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef ARG_UNUSED
/** unused argument placater*/
#define ARG_UNUSED(x) ((void)(x))
#endif

/**
 * @brief Split a line buffer into words, leaving the words null terminated in place in the buffer.
 *
 * @param line pointer to line buffer
 * @param words array of pointers to store start of words
 * @param cnt number of pointers in the words array
 * @return number of words found
 */
RSU_OSAL_INT split_line(RSU_OSAL_CHAR *line, RSU_OSAL_CHAR **words, RSU_OSAL_INT cnt);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

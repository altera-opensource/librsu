/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#ifndef RSU_OSAL_TYPES_H
#define RSU_OSAL_TYPES_H

/**
 *
 * @file RSU_OSAL_types.h
 * @brief contains OS abstraction layer data types for host platform.
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <stdio.h>
#include <stdint.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

/** unsigned 64 bit*/
typedef uint64_t RSU_OSAL_U64;
/** unsigned 32 bit*/
typedef uint32_t RSU_OSAL_U32;
/** unsigned 16 bit*/
typedef uint16_t RSU_OSAL_U16;
/** unsigned 8 bit*/
typedef uint8_t RSU_OSAL_U8;

/** signed 64 bit*/
typedef int64_t RSU_OSAL_S64;
/** signed 32 bit*/
typedef int32_t RSU_OSAL_S32;
/** unsigned 16 bit*/
typedef int16_t RSU_OSAL_S16;
/** unsigned 8 bit*/
typedef int8_t RSU_OSAL_S8;

/** void type*/
typedef void RSU_OSAL_VOID;
/** character data type*/
typedef char RSU_OSAL_CHAR;
/** boolean data type*/
typedef bool RSU_OSAL_BOOL;

/** integer data type*/
typedef int RSU_OSAL_INT;
/** data type to denote offset */
typedef off_t RSU_OSAL_OFFSET;
/** data type to denote size*/
typedef size_t RSU_OSAL_SIZE;

/** mutex object type */
typedef pthread_mutex_t RSU_OSAL_MUTEX;
/** file object type */
typedef FILE RSU_OSAL_FILE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

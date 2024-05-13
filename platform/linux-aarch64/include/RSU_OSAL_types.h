/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

/**
 *
 * @file RSU_OSAL_types.h
 * @brief contains OS abstraction layer data types for linux_aarch64 platform.
 */

#ifndef RSU_OSAL_TYPES_H
#define RSU_OSAL_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */
#include <stdio.h>
#include <stdint.h>
#include <linux/types.h>
#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

/** unsigned 64 bit*/
typedef __u64 RSU_OSAL_U64;
/** unsigned 32 bit*/
typedef __u32 RSU_OSAL_U32;
/** unsigned 16 bit*/
typedef __u16 RSU_OSAL_U16;
/** unsigned 8 bit*/
typedef __u8 RSU_OSAL_U8;

/** signed 64 bit*/
typedef __s64 RSU_OSAL_S64;
/** signed 32 bit*/
typedef __s32 RSU_OSAL_S32;
/** unsigned 16 bit*/
typedef __s16 RSU_OSAL_S16;
/** unsigned 8 bit*/
typedef __s8 RSU_OSAL_S8;

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

/** mutex object type*/
typedef pthread_mutex_t RSU_OSAL_MUTEX;
/** file object type*/
typedef FILE RSU_OSAL_FILE;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

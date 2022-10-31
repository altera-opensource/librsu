/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

/**
 *
 * @file libRSU_OSAL.h
 * @brief OS Abstraction API's used inside LibRSU.
 */

#ifndef LIBRSU_OSAL_H
#define LIBRSU_OSAL_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <RSU_OSAL_types.h>

/**
 * @brief RSU OSAL time for running forever.
 */
#define RSU_TIME_FOREVER (0xFFFFFFFFUL)

/**
 * @brief RSU OSAL time for running once and return immediately.
 *
 */
#define RSU_TIME_NOWAIT (0UL)

/**
 * @brief allocate a buffer in heap memory
 *
 * @param[in] size size of required heap memory in bytes.
 * @return NUll on error or pointer to allocated memory.
 */
RSU_OSAL_VOID *rsu_malloc(RSU_OSAL_SIZE size);

/**
 * @brief free allocated heap memory.
 *
 * @param[in] ptr pointer to heap memory
 * @return Nil
 */
RSU_OSAL_VOID rsu_free(RSU_OSAL_VOID *ptr);

/**
 * @brief seta constant value across a memory range.
 *
 * @param[in] s pointer to start of memory region.
 * @param[in] c value to be set.
 * @param[in] n size of memory region.
 * @return pointer to memory area.
 */
RSU_OSAL_VOID *rsu_memset(RSU_OSAL_VOID *s, RSU_OSAL_U32 c, RSU_OSAL_SIZE n);

/**
 * @brief copy from one memory location to another
 *
 * @param[out] d destination memory location.
 * @param[in] s source memory location.
 * @param[in] n bytes to be copied.
 * @return pointer to destination memory location.
 */
RSU_OSAL_VOID *rsu_memcpy(RSU_OSAL_VOID *d, RSU_OSAL_VOID *s, RSU_OSAL_SIZE n);

/**
 * @brief Initialize a mutex
 *
 * @param[in] mutex pointer to a mutex object of type RSU_OSAL_MUTEX.
 * @return 0 on success, negative number on error.
 */
RSU_OSAL_INT rsu_mutex_init(RSU_OSAL_MUTEX *mutex);

/**
 * @brief Lock a mutex within a time period
 *
 * @note time can also be the below values @ref RSU_TIME_FOREVER and @ref RSU_TIME_NOWAIT.
 *
 * @param[in] mutex pointer to a mutex object of type RSU_OSAL_MUTEX.
 * @param[in] time time period by which the mutex locking should complete. time is in milliseconds.
 * @return 0 on success, negative number on error.
 */
RSU_OSAL_INT rsu_mutex_timedlock(RSU_OSAL_MUTEX *mutex, RSU_OSAL_U32 const time);

/**
 * @brief Unlock a locked mutex
 *
 * @param[in] mutex pointer to a mutex object of type RSU_OSAL_MUTEX.
 * @return 0 on success, negative number on error.
 */
RSU_OSAL_INT rsu_mutex_unlock(RSU_OSAL_MUTEX *mutex);

/**
 * @brief destroy a mutex object and free up resources.
 *
 * @param[in] mutex pointer to a mutex object of type RSU_OSAL_MUTEX.
 * @return 0 on success, negative number on error.
 */
RSU_OSAL_INT rsu_mutex_destroy(RSU_OSAL_MUTEX *mutex);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

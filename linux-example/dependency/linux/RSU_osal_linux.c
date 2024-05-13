/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <libRSU_OSAL.h>
#include <stdlib.h>
/*Predefined header file for mutex and semaphore and time*/
#include <time.h>
#include <string.h>

RSU_OSAL_VOID *rsu_malloc(RSU_OSAL_SIZE size)
{
	return (RSU_OSAL_VOID *)malloc(size);
}

RSU_OSAL_VOID rsu_free(RSU_OSAL_VOID *ptr)
{
	free(ptr);
}

RSU_OSAL_VOID *rsu_memset(RSU_OSAL_VOID *s, RSU_OSAL_U32 c, RSU_OSAL_SIZE n)
{
	return memset(s, c, n);
}

RSU_OSAL_VOID *rsu_memcpy(RSU_OSAL_VOID *d, RSU_OSAL_VOID *s, RSU_OSAL_SIZE n)
{
	return memcpy(d, s, n);
}
/* Mutex initialization */
RSU_OSAL_INT rsu_mutex_init(RSU_OSAL_MUTEX *mutex)
{
	if (mutex == NULL) {
		return -EINVAL;
	}

	RSU_OSAL_INT ret = pthread_mutex_init(mutex, NULL);
	return ret;
}

/* To lock if available else wait based on time */
RSU_OSAL_INT rsu_mutex_timedlock(RSU_OSAL_MUTEX *mutex, RSU_OSAL_U32 const time)
{
	if (mutex == NULL) {
		return -EINVAL;
	}

	if (time == RSU_TIME_FOREVER) {
		return pthread_mutex_lock(mutex);
	} else if (time == RSU_TIME_NOWAIT) {
		return pthread_mutex_trylock(mutex);
	} else {
		struct timespec wait;
		wait.tv_sec = (time / 1000);
		wait.tv_nsec = (time % 1000) * 1000000;
		return pthread_mutex_timedlock(mutex, &wait);
	}
}

/* To release a mutex */
RSU_OSAL_INT rsu_mutex_unlock(RSU_OSAL_MUTEX *mutex)
{
	if (mutex == NULL) {
		return -EINVAL;
	}

	RSU_OSAL_INT ret = pthread_mutex_unlock(mutex);
	return ret;
}

/* Using pthread destroy fucntion */
RSU_OSAL_INT rsu_mutex_destroy(RSU_OSAL_MUTEX *mutex)
{
	if (mutex == NULL) {
		return -EINVAL;
	}

	RSU_OSAL_INT ret = pthread_mutex_destroy(mutex);
	return ret;
}

/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#ifndef LIBRSU_CONTEXT_H
#define LIBRSU_CONTEXT_H

#include <libRSU_OSAL.h>

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

enum rsu_state {
    un_initialized,
    in_progress,
    initialized,
};

struct rsu_context {
    volatile enum rsu_state state;
    RSU_OSAL_MUTEX mutex;
};


#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif

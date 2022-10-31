/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#ifndef RSU_MOCK_UTILS_H
#define RSU_MOCK_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <libRSU_OSAL.h>

RSU_OSAL_VOID swap_bits(RSU_OSAL_CHAR *data, RSU_OSAL_INT size);
RSU_OSAL_U32 swap_endian32(RSU_OSAL_U32 val);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

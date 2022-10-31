/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

/**
 *
 * @file RSU_plat_crc32.h
 * @brief contains CRC functionality that each platform must populate.
 */

#ifndef RSU_PLAT_CRC32_H
#define RSU_PLAT_CRC32_H

#include <libRSU_OSAL.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief returns a crc32 checksum calculation for the given buffer.
 *
 * @param[in] crc initial crc value.
 * @param[in] data pointer to the buffer.
 * @param[in] len length of the buffer.
 * @return computed crc32 value.
 */
RSU_OSAL_U32 rsu_crc32(RSU_OSAL_U32 crc, const RSU_OSAL_U8 *data, RSU_OSAL_SIZE len);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

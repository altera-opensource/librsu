/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <hal/RSU_plat_crc32.h>
#include <zlib.h>

RSU_OSAL_U32 rsu_crc32(RSU_OSAL_U32 crc, const RSU_OSAL_U8 *data, RSU_OSAL_SIZE len)
{
    return crc32(crc,data,len);
}

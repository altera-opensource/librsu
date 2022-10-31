/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include "rsu_mock_utils.h"
#include <string.h>

RSU_OSAL_VOID swap_bits(RSU_OSAL_CHAR *data, RSU_OSAL_INT len)
{
	RSU_OSAL_INT x, y;
	RSU_OSAL_CHAR tmp;

	for (x = 0; x < len; x++) {
		tmp = 0;
		for (y = 0; y < 8; y++) {
			tmp <<= 1;
			if (data[x] & 1) {
				tmp |= 1;
			}
			data[x] >>= 1;
		}
		data[x] = tmp;
	}
}

RSU_OSAL_U32 swap_endian32(RSU_OSAL_U32 val)
{
	RSU_OSAL_U32 rtn;
	RSU_OSAL_CHAR *from = (RSU_OSAL_CHAR *)&val;
	RSU_OSAL_CHAR *to = (RSU_OSAL_CHAR *)&rtn;
	RSU_OSAL_INT x;

	for (x = 0; x < 4; x++) {
		to[x] = from[3 - x];
	}

	return rtn;
}

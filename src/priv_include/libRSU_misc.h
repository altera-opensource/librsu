/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#ifndef __LIBRSU_MISC_H__
#define __LIBRSU_MISC_H__

// #include "librsu_ll.h"
#include <libRSU_OSAL.h>
#include <libRSU_hl_intf.h>

RSU_OSAL_BOOL librsu_misc_is_rsvd_name(RSU_OSAL_CHAR *name);

RSU_OSAL_BOOL librsu_misc_is_slot(struct librsu_hl_intf *intf, RSU_OSAL_INT part_num);
RSU_OSAL_INT librsu_misc_slot2part(struct librsu_hl_intf *intf, RSU_OSAL_INT slot);

RSU_OSAL_VOID swap_bits(RSU_OSAL_CHAR *data, RSU_OSAL_INT size);
RSU_OSAL_U32 swap_endian32(RSU_OSAL_U32 val);

RSU_OSAL_VOID SAFE_STRCPY(RSU_OSAL_CHAR *dst, RSU_OSAL_INT dsz, RSU_OSAL_CHAR *src, RSU_OSAL_INT ssz);

#endif

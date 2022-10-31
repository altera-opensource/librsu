/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#ifndef LIBRSU_CFG_H
#define LIBRSU_CFG_H

#include <libRSU_OSAL.h>
#include <libRSU_hl_intf.h>
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

RSU_OSAL_INT librsu_cfg_parse(RSU_OSAL_CHAR *filename, struct librsu_hl_intf **intf);
RSU_OSAL_INT librsu_common_cfg_parse(RSU_OSAL_CHAR *filename, struct librsu_ll_intf *intf);
RSU_OSAL_VOID librsu_cfg_reset(RSU_OSAL_VOID);

RSU_OSAL_INT librsu_cfg_writeprotected(RSU_OSAL_INT slot);
RSU_OSAL_INT librsu_cfg_spt_checksum_enabled(RSU_OSAL_VOID);
struct librsu_ll_intf *librsu_get_ll_inf(RSU_OSAL_VOID);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

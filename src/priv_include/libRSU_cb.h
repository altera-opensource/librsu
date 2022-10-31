/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#ifndef __LIBRSU_CB_H__
#define __LIBRSU_CB_H__

#include <libRSU.h>
#include <libRSU_OSAL.h>
#include <libRSU_hl_intf.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

RSU_OSAL_INT librsu_cb_file_init(RSU_OSAL_CHAR *filename);
RSU_OSAL_VOID librsu_cb_file_cleanup(RSU_OSAL_VOID);
RSU_OSAL_INT librsu_cb_file(RSU_OSAL_VOID *buf, RSU_OSAL_INT len);

RSU_OSAL_INT librsu_cb_buf_init(RSU_OSAL_VOID *buf, RSU_OSAL_INT size);
RSU_OSAL_VOID librsu_cb_buf_cleanup(RSU_OSAL_VOID);
RSU_OSAL_INT librsu_cb_buf(RSU_OSAL_VOID *buf, RSU_OSAL_INT len);

RSU_OSAL_INT librsu_cb_program_common(struct librsu_hl_intf *intf, RSU_OSAL_INT slot,
				      rsu_data_callback callback, RSU_OSAL_INT rawdata);

RSU_OSAL_INT librsu_cb_verify_common(struct librsu_hl_intf *intf, RSU_OSAL_INT slot,
				     rsu_data_callback callback, RSU_OSAL_INT rawdata);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

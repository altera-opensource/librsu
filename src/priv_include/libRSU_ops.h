/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#ifndef LIBRSU_QSPI_H
#define LIBRSU_QSPI_H

#ifdef __cplusplus
extern "C"
{
#endif  /* __cplusplus */

#include <libRSU_OSAL.h>
#include <libRSU_ll_intf.h>
#include <libRSU_hl_intf.h>

RSU_OSAL_INT rsu_qspi_open(struct librsu_ll_intf *intf, struct librsu_hl_intf **hl_ptr);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif

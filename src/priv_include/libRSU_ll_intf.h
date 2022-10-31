/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#ifndef LIBRSU_LL_INTF_H
#define LIBRSU_LL_INTF_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <hal/RSU_plat_qspi.h>
#include <hal/RSU_plat_mailbox.h>
#include <hal/RSU_plat_file.h>
#include <hal/RSU_plat_misc.h>
#include <hal/RSU_plat_crc32.h>

struct librsu_ll_intf {
	struct qspi_ll_intf qspi;
	struct mbox_ll_intf mbox;
	struct filesys_ll_intf file;
	struct rsu_ll_misc misc;
	RSU_OSAL_U32 writeprotect;
	RSU_OSAL_U32 spt_checksum_enabled;
};

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <hal/RSU_plat_mailbox.h>
#include <utils/RSU_logging.h>
#include <utils/RSU_utils.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Mocking function for get_rsu_status */
RSU_OSAL_INT plat_mbox_get_rsu_status_mock(struct mbox_status_info *data)
{
	if(data == NULL){
		return -EINVAL;
	}

	data->version = 0x0808;
	return 0;
}

/* Mocking function for send_rsu_update */
RSU_OSAL_INT plat_mbox_send_rsu_update_mock(RSU_OSAL_U64 addr)
{
	return 0;
}

/* Mocking function for get_spt_addresses */
RSU_OSAL_INT plat_mbox_get_spt_addresses_mock(struct mbox_data_rsu_spt_address *data)
{
	if (data == NULL) {
		return -EINVAL;
	}
	// data->spt0_address = 0x00310000;
	// data->spt1_address = 0x00318000;
	data->spt0_address = 0x0000;
	data->spt1_address = 0x8000;
	return 0;
}

/* Mocking function for rsu_notify */
RSU_OSAL_INT plat_mbox_rsu_notify_mock(RSU_OSAL_U32 notify)
{
	return 0;
}

/* Mocking function for terminate */
RSU_OSAL_INT plat_mbox_terminate_mock(RSU_OSAL_VOID)
{
	return 0;
}

RSU_OSAL_INT plat_mbox_init(struct mbox_ll_intf *mbox, RSU_OSAL_CHAR *config_file)
{
	if (!mbox || !config_file) {
		return -EINVAL;
	}
	ARG_UNUSED(config_file);
	mbox->get_rsu_status = plat_mbox_get_rsu_status_mock;
	mbox->send_rsu_update = plat_mbox_send_rsu_update_mock;
	mbox->get_spt_addresses = plat_mbox_get_spt_addresses_mock;
	mbox->rsu_notify = plat_mbox_rsu_notify_mock;
	mbox->terminate = plat_mbox_terminate_mock;
	return 0;
}

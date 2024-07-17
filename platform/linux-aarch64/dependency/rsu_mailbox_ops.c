/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include "rsu_linux_utils.h"
#include <hal/RSU_plat_mailbox.h>
#include <utils/RSU_logging.h>
#include <utils/RSU_utils.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static RSU_OSAL_CHAR rsu_dev_local[RSU_DEV_BUF_SIZE + 1] = DEFAULT_RSU_DEV;


static RSU_OSAL_INT plat_mbox_get_rsu_status(struct mbox_status_info *data)
{
	if(data == NULL){
		return -EINVAL;
	}

	RSU_OSAL_INT ret = 0;

	ret = get_devattr(rsu_dev_local, "version", &(data->version));
	if(ret != 0) {
		return ret;
	}

	ret = get_devattr(rsu_dev_local, "state", &(data->state));
	if(ret != 0) {
		return ret;
	}

	ret = get_devattr(rsu_dev_local, "current_image", &(data->current_image));
	if(ret != 0) {
		return ret;
	}

	ret = get_devattr(rsu_dev_local, "fail_image", &(data->fail_image));
	if(ret != 0) {
		return ret;
	}

	ret = get_devattr(rsu_dev_local, "error_location", &(data->error_location));
	if(ret != 0) {
		return ret;
	}

	ret = get_devattr(rsu_dev_local, "error_details", &(data->error_details));
	if(ret != 0) {
		return ret;
	}

	ret = get_devattr(rsu_dev_local, "retry_counter", &(data->retry_counter));
	if(ret != 0) {
		return ret;
	}

	return 0;
}


static RSU_OSAL_INT plat_mbox_send_rsu_update(RSU_OSAL_U64 addr)
{
	RSU_OSAL_INT ret = 0;

	ret = put_devattr(rsu_dev_local, "reboot_image", addr);
	if(ret != 0) {
		return ret;
	}
	return 0;
}

static RSU_OSAL_INT plat_mbox_get_spt_addresses(struct mbox_data_rsu_spt_address *data)
{
	if (data == NULL) {
		return -EINVAL;
	}

	RSU_OSAL_INT ret = 0;

	ret = get_devattr(rsu_dev_local, "spt0_address", &(data->spt0_address));
	if(ret != 0) {
		return ret;
	}

	ret = get_devattr(rsu_dev_local, "spt1_address", &(data->spt1_address));
	if(ret != 0) {
		return ret;
	}

	/*start the offset from spt0_address for linux*/
	data->spt1_address = data->spt1_address - data->spt0_address;
	data->spt0_address = 0;

	return 0;
}

static RSU_OSAL_INT plat_mbox_rsu_notify(RSU_OSAL_U32 notify)
{
	RSU_OSAL_INT ret = 0;

	ret = put_devattr(rsu_dev_local, "notify", notify);
	if(ret != 0) {
		return ret;
	}

	return 0;
}

static RSU_OSAL_INT plat_mbox_terminate(RSU_OSAL_VOID)
{
	strncpy(rsu_dev_local, DEFAULT_RSU_DEV, RSU_DEV_BUF_SIZE);
	return 0;
}

RSU_OSAL_INT plat_mbox_init(struct mbox_ll_intf *mbox, RSU_OSAL_CHAR *config_file)
{
	if (!mbox || !config_file) {
		return -EINVAL;
	}

	RSU_OSAL_FILE *file;
	RSU_OSAL_CHAR line[RSU_DEV_BUF_SIZE], *argv[NUM_ARGS];
	RSU_OSAL_INT argc;
	RSU_OSAL_U32 linenum;

	file = fopen(config_file, "r");
	if (!file) {
		return -ENOENT;
	}

	linenum = 0;
	while (fgets(line, RSU_DEV_BUF_SIZE, file) != NULL) {
		linenum++;
		argc = split_line(line, argv, NUM_ARGS);
		if (argc != 2) {
			continue;
		}

		if (strncmp(argv[0], "rsu-dev", RSU_DEV_BUF_SIZE) == 0) {
			strncpy(rsu_dev_local, argv[1], RSU_DEV_BUF_SIZE);
			break;
		}
	}

	RSU_LOG_DBG("rsu-dev is %.*s for mailbox operations\n", RSU_DEV_BUF_SIZE, rsu_dev_local);
	fclose(file);

	mbox->get_rsu_status = plat_mbox_get_rsu_status;
	mbox->send_rsu_update = plat_mbox_send_rsu_update;
	mbox->get_spt_addresses = plat_mbox_get_spt_addresses;
	mbox->rsu_notify = plat_mbox_rsu_notify;
	mbox->terminate = plat_mbox_terminate;
	return 0;
}

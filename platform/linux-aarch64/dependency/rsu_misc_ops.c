/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include "rsu_linux_utils.h"
#include <hal/RSU_plat_misc.h>
#include <utils/RSU_utils.h>
#include <utils/RSU_logging.h>
#include <utils/RSU_utils.h>
#include <string.h>

static RSU_OSAL_CHAR rsu_dev_local[RSU_DEV_BUF_SIZE + 1] = DEFAULT_RSU_DEV;

static RSU_OSAL_INT rsu_get_dcmf_status(struct rsu_dcmf_status *data)
{
	if (data == NULL) {
		return -EINVAL;
	}

	RSU_OSAL_INT ret = 0;
	RSU_OSAL_U64 value;

	ret = get_devattr(rsu_dev_local, "dcmf0_status", &value);
	if (ret != 0) {
		return ret;
	} else {
		data->dcmf[0] = (RSU_OSAL_U32)value;
	}

	ret = get_devattr(rsu_dev_local, "dcmf1_status", &value);
	if (ret != 0) {
		return ret;
	} else {
		data->dcmf[1] = (RSU_OSAL_U32)value;
	}

	ret = get_devattr(rsu_dev_local, "dcmf2_status", &value);
	if (ret != 0) {
		return ret;
	} else {
		data->dcmf[2] = (RSU_OSAL_U32)value;
	}

	ret = get_devattr(rsu_dev_local, "dcmf3_status", &value);
	if (ret != 0) {
		return ret;
	} else {
		data->dcmf[3] = (RSU_OSAL_U32)value;
	}

	return 0;
}

static RSU_OSAL_INT rsu_get_max_retry_count(RSU_OSAL_U8 *rsu_max_retry)
{
	if (rsu_max_retry == NULL) {
		return -EINVAL;
	}

	RSU_OSAL_INT ret = 0;
	RSU_OSAL_U64 max_retry;

	ret = get_devattr(rsu_dev_local, "max_retry", &max_retry);
	if (ret != 0) {
		return ret;
	}

	if (max_retry > RSU_MAX_RETRY_LIMIT) {
		return -EINVAL;
	}

	*rsu_max_retry = max_retry;

	return 0;
}

static RSU_OSAL_INT rsu_get_dcmf_version(struct rsu_dcmf_version *version)
{
	if (version == NULL) {
		return -EINVAL;
	}

	RSU_OSAL_INT ret = 0;
	RSU_OSAL_U64 value;

	ret = get_devattr(rsu_dev_local, "dcmf0", &value);
	if (ret != 0) {
		return ret;
	} else {
		version->dcmf[0] = (RSU_OSAL_U32)value;
	}

	ret = get_devattr(rsu_dev_local, "dcmf1", &value);
	if (ret != 0) {
		return ret;
	} else {
		version->dcmf[1] = (RSU_OSAL_U32)value;
	}

	ret = get_devattr(rsu_dev_local, "dcmf2", &value);
	if (ret != 0) {
		return ret;
	} else {
		version->dcmf[2] = (RSU_OSAL_U32)value;
	}

	ret = get_devattr(rsu_dev_local, "dcmf3", &value);
	if (ret != 0) {
		return ret;
	} else {
		version->dcmf[3] = (RSU_OSAL_U32)value;
	}

	return 0;
}

static RSU_OSAL_INT terminate(RSU_OSAL_VOID)
{
	strncpy(rsu_dev_local, DEFAULT_RSU_DEV, RSU_DEV_BUF_SIZE);
	return 0;
}

RSU_OSAL_INT plat_rsu_misc_init(struct rsu_ll_misc *misc_intf, RSU_OSAL_CHAR *config_file)
{
	if (!misc_intf || !config_file) {
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
	RSU_LOG_DBG("rsu-dev is %.*s for misc operations\n", RSU_DEV_BUF_SIZE, rsu_dev_local);
	fclose(file);

	misc_intf->rsu_get_dcmf_status = rsu_get_dcmf_status;
	misc_intf->rsu_get_dcmf_version = rsu_get_dcmf_version;
	misc_intf->rsu_get_max_retry_count = rsu_get_max_retry_count;
	misc_intf->terminate = terminate;

	return 0;
}

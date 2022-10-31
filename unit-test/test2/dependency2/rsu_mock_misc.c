/*
 * Copyright (C) 2023-2024 Intel Corporation
 * SPDX-License-Identifier: MIT-0
 */

#include <hal/RSU_plat_misc.h>
#include <utils/RSU_utils.h>

RSU_OSAL_INT rsu_get_dcmf_status(struct rsu_dcmf_status *data)
{
	if (data == NULL) {
		return -EINVAL;
	}

	return 0;
}

RSU_OSAL_INT rsu_get_max_retry_count(RSU_OSAL_U8 *rsu_max_retry)
{
	if (rsu_max_retry == NULL) {
		return -EINVAL;
	}

	return 0;
}

RSU_OSAL_INT rsu_get_dcmf_version(struct rsu_dcmf_version *version)
{
	if (version == NULL) {
		return -EINVAL;
	}

	return 0;
}

RSU_OSAL_INT terminate(RSU_OSAL_VOID)
{
	return 0;
}

RSU_OSAL_INT plat_rsu_misc_init(struct rsu_ll_misc *misc_intf, RSU_OSAL_CHAR *config_file)
{
	if (!misc_intf || !config_file) {
		return -EINVAL;
	}

	ARG_UNUSED(config_file);
	misc_intf->rsu_get_dcmf_status = rsu_get_dcmf_status;
	misc_intf->rsu_get_dcmf_version = rsu_get_dcmf_version;
	misc_intf->rsu_get_max_retry_count = rsu_get_max_retry_count;
	misc_intf->terminate = terminate;

	return 0;
}
